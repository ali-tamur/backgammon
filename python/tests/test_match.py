"""run_match through the real engine (via bazel), and git_commit on a scratch repo."""

import subprocess

import pytest

from bg.db import Db
from bg.match import git_commit, repo_root, run_match
from proto import config_pb2

TEST_MODEL = repo_root() / "cc" / "src" / "testdata" / "test_model.onnx"


@pytest.fixture
def db(tmp_path):
    db = Db.open(tmp_path / "test.db")
    yield db
    db.close()


def add_network(db: Db, onnx: bytes) -> int:
    return db.add_network(
        onnx, config_pb2.NetworkConfig(), input_encoding="TEST", output_layout="TEST", git_commit="abc123"
    )


def status(db: Db, match_id: int) -> str:
    return db.conn.execute("SELECT status FROM match WHERE id = ?", (match_id,)).fetchone()[0]


def test_random_match_is_stored_and_reproducible(db):
    random = db.player("RANDOM")
    first = run_match(db, random, random, 4, purpose="TRAINING", seed=7, allow_dirty=True)
    again = run_match(db, random, random, 4, purpose="TRAINING", seed=7, allow_dirty=True)
    other = run_match(db, random, random, 4, purpose="TRAINING", seed=8, allow_dirty=True)

    assert status(db, first) == "COMPLETE"
    assert db.summary(first).games == 4
    games = list(db.game_records(first))
    assert games == list(db.game_records(again))
    assert games != list(db.game_records(other))
    # ALTERNATE: game i's first player alternates with i.
    assert [game.white_first for game in db.games(first)] == [True, False, True, False]

    seed, commit, config = db.conn.execute(
        "SELECT seed, git_commit, config FROM match WHERE id = ?", (first,)
    ).fetchone()
    assert seed == 7
    assert commit
    assert "--seed=7" in config and "--n_games=4" in config


def test_nn_player_plays(db):
    nn = db.player("NN", add_network(db, TEST_MODEL.read_bytes()), config_pb2.PlayerConfig(temperature=0))
    random = db.player("RANDOM")
    match = run_match(db, nn, random, 2, purpose="EVALUATION", seed=1, allow_dirty=True)
    assert status(db, match) == "COMPLETE"
    assert db.summary(match).games == 2
    config = db.conn.execute("SELECT config FROM match WHERE id = ?", (match,)).fetchone()[0]
    assert "--w_temperature=0.0" in config


def test_engine_failure_marks_the_match_failed(db):
    broken = db.player("NN", add_network(db, b"not an onnx model"), config_pb2.PlayerConfig(temperature=0))
    random = db.player("RANDOM")
    with pytest.raises(RuntimeError, match="engine failed"):
        run_match(db, broken, random, 1, purpose="EVALUATION", seed=1, allow_dirty=True)
    (match,) = db.conn.execute("SELECT id FROM match").fetchone()
    assert status(db, match) == "FAILED"
    assert db.conn.execute("SELECT COUNT(*) FROM game").fetchone()[0] == 0


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
