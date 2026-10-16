"""run_match through the real engine (via bazel), and git_commit on a scratch repo."""

import subprocess

import pytest

from bg.db import Game, Match, Network, Player, to_text
from bg.match import git_commit, repo_root, run_match
from proto import config_pb2

TEST_MODEL = repo_root() / "cc" / "src" / "testdata" / "test_model.onnx"


def nn_player(onnx: bytes) -> Player:
    network = Network.create(
        onnx=onnx,
        config=to_text(config_pb2.NetworkConfig()),
        input_encoding="TEST",
        output_layout="TEST",
        git_commit="abc123",
    )
    return Player.find_or_create("NN", network, config_pb2.PlayerConfig(temperature=0))


def test_random_match_is_stored_and_reproducible(db):
    random = Player.find_or_create("RANDOM")
    first = run_match(random, random, 4, purpose="TRAINING", seed=7, allow_dirty=True)
    again = run_match(random, random, 4, purpose="TRAINING", seed=7, allow_dirty=True)
    other = run_match(random, random, 4, purpose="TRAINING", seed=8, allow_dirty=True)

    first = Match.get_by_id(first.id)
    assert first.status == "COMPLETE"
    assert first.summary().games == 4
    games = list(first.game_records())
    assert games == list(again.game_records())
    assert games != list(other.game_records())
    # ALTERNATE: game i's first player alternates with i.
    assert [game.white_first for game in first.game_protos()] == [True, False, True, False]
    assert first.seed == 7
    assert first.git_commit
    assert "--seed=7" in first.config and "--n_games=4" in first.config


def test_nn_player_plays(db):
    match = run_match(
        nn_player(TEST_MODEL.read_bytes()),
        Player.find_or_create("RANDOM"),
        2,
        purpose="EVALUATION",
        seed=1,
        allow_dirty=True,
    )
    match = Match.get_by_id(match.id)
    assert match.status == "COMPLETE"
    assert match.summary().games == 2
    assert "--w_temperature=0.0" in match.config


def test_engine_failure_marks_the_match_failed(db):
    with pytest.raises(RuntimeError, match="engine failed"):
        run_match(
            nn_player(b"not an onnx model"),
            Player.find_or_create("RANDOM"),
            1,
            purpose="EVALUATION",
            seed=1,
            allow_dirty=True,
        )
    (match,) = Match.select()
    assert match.status == "FAILED"
    assert Game.select().count() == 0


def git(repo, *args: str) -> str:
    return subprocess.run(["git", *args], cwd=repo, capture_output=True, text=True, check=True).stdout.strip()


@pytest.fixture
def repo(tmp_path):
    git(tmp_path, "init", "-q")
    git(tmp_path, "config", "user.email", "test@example.com")
    git(tmp_path, "config", "user.name", "test")
    (tmp_path / "tracked.txt").write_text("1")
    git(tmp_path, "add", "tracked.txt")
    git(tmp_path, "commit", "-q", "-m", "first")
    return tmp_path


def test_git_commit_clean_tree(repo):
    head = git(repo, "rev-parse", "HEAD")
    assert git_commit(repo, allow_dirty=False) == head
    (repo / "untracked.txt").write_text("ignored")
    assert git_commit(repo, allow_dirty=False) == head


def test_git_commit_dirty_tree(repo):
    head = git(repo, "rev-parse", "HEAD")
    (repo / "tracked.txt").write_text("2")
    with pytest.raises(RuntimeError, match="uncommitted changes"):
        git_commit(repo, allow_dirty=False)
    assert git_commit(repo, allow_dirty=True) == head + "-dirty"
