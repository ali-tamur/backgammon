import sqlite3
import struct

import pytest

from bg.db import SCHEMA_VERSION, Db, Summary, read_games
from proto import config_pb2, game_pb2


@pytest.fixture
def db(tmp_path):
    db = Db.open(tmp_path / "test.db")
    yield db
    db.close()


def make_game(white_first: bool, final_score: int, n_turns: int) -> bytes:
    game = game_pb2.Game(white_first=white_first, final_score=final_score)
    for _ in range(n_turns):
        game.turns.add(dice=63)
    return game.SerializeToString()


def add_network(db: Db) -> int:
    return db.add_network(
        b"onnx bytes",
        config_pb2.NetworkConfig(),
        input_encoding="ONE_HOT_248_MOVER",
        output_layout="EXCLUSIVE_4",
        git_commit="abc123",
    )


def add_match(db: Db, n_games: int) -> int:
    random = db.player("RANDOM")
    return db.create_match(
        random,
        random,
        purpose="TRAINING",
        first_move="ALTERNATE",
        seed=1,
        n_games=n_games,
        git_commit="abc123",
        config=config_pb2.MatchConfig(),
    )


def test_open_sets_schema_version(tmp_path):
    path = tmp_path / "test.db"
    Db.open(path).close()
    assert sqlite3.connect(path).execute("PRAGMA user_version").fetchone()[0] == SCHEMA_VERSION


def test_open_rejects_other_schema_version(tmp_path):
    path = tmp_path / "test.db"
    conn = sqlite3.connect(path)
    conn.execute("PRAGMA user_version = 999")
    conn.close()
    with pytest.raises(RuntimeError, match="regenerate"):
        Db.open(path)


def test_player_is_found_not_duplicated(db):
    network = add_network(db)
    first = db.player("NN", network, config_pb2.PlayerConfig(temperature=0.1))
    second = db.player("NN", network, config_pb2.PlayerConfig(temperature=0.1))
    other = db.player("NN", network, config_pb2.PlayerConfig(temperature=0))
    assert first == second
    assert first != other
    assert db.player("RANDOM") == db.player("RANDOM")


def test_player_settings_round_trip(db):
    network = add_network(db)
    player = db.player("NN", network, config_pb2.PlayerConfig(temperature=0.25))
    assert db.player_settings(player) == ("NN", network, config_pb2.PlayerConfig(temperature=0.25))


def test_random_player_takes_no_config(db):
    with pytest.raises(ValueError):
        db.player("RANDOM", config=config_pb2.PlayerConfig(temperature=0.1))


def test_nn_player_needs_a_network(db):
    with pytest.raises(sqlite3.IntegrityError):
        db.player("NN")


def test_unknown_network_is_rejected(db):
    with pytest.raises(sqlite3.IntegrityError):
        db.player("NN", network_id=42)


def test_complete_match_and_summary(db):
    match = add_match(db, 3)
    db.complete_match(match, [make_game(True, 1, 40), make_game(False, -2, 60), make_game(True, 3, 80)])
    assert db.summary(match) == Summary(
        games=3,
        white_points=4,
        black_points=2,
        white_wins=2,
        black_wins=1,
        white_gammons=1,
        black_gammons=1,
        avg_turns=60.0,
    )
    assert db.conn.execute("SELECT status FROM match WHERE id = ?", (match,)).fetchone()[0] == "COMPLETE"
    assert [game.final_score for game in db.games(match)] == [1, -2, 3]


def test_wrong_game_count_stores_nothing(db):
    match = add_match(db, 3)
    with pytest.raises(ValueError):
        db.complete_match(match, [make_game(True, 1, 40)])
    assert db.conn.execute("SELECT COUNT(*) FROM game").fetchone()[0] == 0
    assert db.conn.execute("SELECT status FROM match WHERE id = ?", (match,)).fetchone()[0] == "RUNNING"


def test_read_games(tmp_path):
    games = [make_game(True, 1, 5), make_game(False, -1, 7)]
    path = tmp_path / "games.pb"
    path.write_bytes(b"".join(struct.pack("<I", len(g)) + g for g in games))
    assert list(read_games(path)) == games


def test_read_games_rejects_truncated_file(tmp_path):
    game = make_game(True, 1, 5)
    path = tmp_path / "games.pb"
    path.write_bytes(struct.pack("<I", len(game)) + game[:-1])
    with pytest.raises(ValueError, match="truncated"):
        list(read_games(path))
