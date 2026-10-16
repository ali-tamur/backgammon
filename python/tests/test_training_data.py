import numpy as np
import pytest

import bg.training_data
from bg.db import Db, read_games
from bg.training_data import ARRAYS, build, export_games, load, training_data
from proto import config_pb2, game_pb2

START = [2, 0, 0, 0, 0, -5, 0, -3, 0, 0, 0, 5, -5, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, -2, 0, 0, 0, 0]


def flip(board: list[int]) -> list[int]:
    """The same position seen by the other player (mirrors flip() in cc/src/configs.h)."""
    points = [-c for c in reversed(board[:24])]
    return points + [-board[25], -board[24], -board[27], -board[26]]


def make_game(white_first: bool, final_score: int, boards: list[list[int]]) -> bytes:
    game = game_pb2.Game(white_first=white_first, final_score=final_score)
    game.start.cell.extend(START)
    for board in boards:
        game.turns.add(dice=63).after.cell.extend(board)
    return game.SerializeToString()


@pytest.fixture
def db(tmp_path):
    db = Db.open(tmp_path / "test.db")
    yield db
    db.close()


def add_match(db: Db, games: list[bytes]) -> int:
    random = db.player("RANDOM")
    match_id = db.create_match(
        random,
        random,
        purpose="TRAINING",
        first_move="ALTERNATE",
        seed=1,
        n_games=len(games),
        git_commit="abc123",
        config=config_pb2.MatchConfig(),
    )
    db.complete_match(match_id, games)
    return match_id


def test_export_games_keeps_match_order(db, tmp_path):
    first = [make_game(True, 1, [START])]
    second = [make_game(False, -1, [START]), make_game(True, 2, [START])]
    m1 = add_match(db, first)
    m2 = add_match(db, second)
    path = tmp_path / "games.pb"
    assert export_games(db, [m2, m1], path) == 3
    assert list(read_games(path)) == second + first


def test_incomplete_match_cannot_be_exported(db, tmp_path):
    random = db.player("RANDOM")
    match_id = db.create_match(
        random,
        random,
        purpose="TRAINING",
        first_move="ALTERNATE",
        seed=1,
        n_games=1,
        git_commit="abc123",
        config=config_pb2.MatchConfig(),
    )
    with pytest.raises(ValueError, match="not complete"):
        export_games(db, [match_id], tmp_path / "games.pb")


def test_converter_flips_black_and_labels_for_the_mover(db, tmp_path):
    """Runs the real C++ converter (via bazel) on a two-turn game."""
    after_black = list(START)
    after_black[23], after_black[17] = -1, -1
    after_white = list(after_black)
    after_white[0], after_white[4] = 1, 1
    # Black moves first, white wins a gammon.
    match_id = add_match(db, [make_game(False, 2, [after_black, after_white])])

    build(db, [match_id], tmp_path / "arrays")
    arrays = load(tmp_path / "arrays")

    assert arrays["boards"].dtype == np.int8
    assert arrays["boards"].tolist() == [flip(after_black), after_white]
    assert arrays["labels"].tolist() == [3, 1]  # opponent gammon, mover gammon
    assert arrays["points"].tolist() == [-2, 2]
    assert arrays["turn"].tolist() == [0, 1]
    assert arrays["n_turns"].tolist() == [2, 2]
    assert arrays["game"].tolist() == [0, 0]


class FakeBuild:
    """Stands in for the converter: writes one-element arrays and counts calls."""

    def __init__(self, fail: bool = False):
        self.calls = 0
        self.fail = fail

    def __call__(self, db, match_ids, out_dir):
        self.calls += 1
        for name in ARRAYS:
            np.save(out_dir / f"{name}.npy", np.array([len(match_ids)]))
        if self.fail:
            raise KeyboardInterrupt


@pytest.fixture
def fake(monkeypatch, tmp_path):
    """training_data() with a fake converter, a fixed commit and a cache in tmp_path."""
    state = {"commit": "c0ffee"}
    build = FakeBuild()
    monkeypatch.setattr(bg.training_data, "build", build)
    monkeypatch.setattr(bg.training_data, "repo_root", lambda: tmp_path)
    monkeypatch.setattr(bg.training_data, "git_commit", lambda root, allow_dirty: state["commit"])
    return build, state, tmp_path / "cache"


def test_cache_is_reused(fake):
    build, _, cache = fake
    first = training_data(None, [1, 2], cache)
    second = training_data(None, [1, 2], cache)
    assert build.calls == 1
    assert first["labels"].tolist() == second["labels"].tolist() == [2]
    (entry,) = cache.iterdir()
    assert (entry / "inputs.txt").read_text() == "matches: [1, 2]\ncommit: c0ffee\n"


def test_cache_key_covers_matches_and_commit(fake):
    build, state, cache = fake
    training_data(None, [1, 2], cache)
    training_data(None, [2, 1], cache)  # order matters: it is the order of the examples
    state["commit"] = "beef"
    training_data(None, [1, 2], cache)
    assert build.calls == 3
    assert len(list(cache.iterdir())) == 3


def test_uncommitted_changes_are_never_cached(fake):
    build, state, cache = fake
    state["commit"] = "c0ffee-dirty"
    training_data(None, [1], cache)
    training_data(None, [1], cache)
    assert build.calls == 2
    assert not cache.exists()


def test_interrupted_build_leaves_no_cache_entry(fake):
    build, _, cache = fake
    build.fail = True
    with pytest.raises(KeyboardInterrupt):
        training_data(None, [1], cache)
    assert list(cache.iterdir()) == []
    build.fail = False
    assert training_data(None, [1], cache)["labels"].tolist() == [1]
