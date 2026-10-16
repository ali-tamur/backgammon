import sqlite3

import peewee
import pytest
from peewee import fn

from bg.db import SCHEMA_VERSION, Game, Match, Network, Player, Summary, open_db, to_text
from bg.game_log import read_games, write_games
from proto import config_pb2, game_pb2


def make_game(white_first: bool, final_score: int, n_turns: int) -> bytes:
    game = game_pb2.Game(white_first=white_first, final_score=final_score)
    for _ in range(n_turns):
        game.turns.add(dice=63)
    return game.SerializeToString()


def add_network(**kwargs) -> Network:
    return Network.create(
        onnx=b"onnx bytes",
        config=to_text(config_pb2.NetworkConfig()),
        input_encoding="ONE_HOT_248_MOVER",
        output_layout="EXCLUSIVE_4",
        git_commit="abc123",
        **kwargs,
    )


def add_match(n_games: int) -> Match:
    random = Player.find_or_create("RANDOM")
    return Match.create(
        white=random,
        black=random,
        purpose="TRAINING",
        first_move="ALTERNATE",
        seed=1,
        n_games=n_games,
        git_commit="abc123",
        config=to_text(config_pb2.MatchConfig()),
    )


def test_open_sets_schema_version(tmp_path):
    path = tmp_path / "test.db"
    with open_db(path):
        pass
    assert sqlite3.connect(path).execute("PRAGMA user_version").fetchone()[0] == SCHEMA_VERSION


def test_open_rejects_other_schema_version(tmp_path):
    path = tmp_path / "test.db"
    conn = sqlite3.connect(path)
    conn.execute("PRAGMA user_version = 999")
    conn.close()
    with pytest.raises(RuntimeError, match="regenerate"):
        with open_db(path):
            pass
    with open_db(tmp_path / "other.db"):  # the failed open left nothing bound
        pass


def test_reopening_keeps_the_data(tmp_path):
    path = tmp_path / "test.db"
    with open_db(path):
        network = add_network()
    with open_db(path):
        assert Network.get_by_id(network.id).onnx == b"onnx bytes"


def test_closed_database_is_not_reopened_silently(tmp_path):
    with open_db(tmp_path / "test.db"):
        add_network()
    with pytest.raises(peewee.InterfaceError, match="must be initialized"):
        Network.select().count()


def test_closed_even_after_an_error(tmp_path):
    with pytest.raises(ZeroDivisionError):
        with open_db(tmp_path / "test.db"):
            1 / 0
    with pytest.raises(peewee.InterfaceError):
        Network.select().count()


def test_only_one_database_at_a_time(db, tmp_path):
    with pytest.raises(RuntimeError, match="already open"):
        with open_db(tmp_path / "other.db"):
            pass
    assert Network.select().count() == 0  # the open one is untouched


def test_network_round_trip(db):
    parent = add_network()
    child = add_network(parent=parent, checkpoint=b"weights", note="second")
    child = Network.get_by_id(child.id)
    assert child.parent == parent
    assert list(parent.children) == [child]
    assert (child.onnx, child.checkpoint, child.note) == (b"onnx bytes", b"weights", "second")
    assert child.config_proto == config_pb2.NetworkConfig()
    assert child.created


def test_ids_are_never_reused(db):
    first = add_match(1)
    first.delete_instance()
    assert add_match(1).id == first.id + 1


def test_player_is_found_not_duplicated(db):
    network = add_network()
    first = Player.find_or_create("NN", network, config_pb2.PlayerConfig(temperature=0.1))
    second = Player.find_or_create("NN", network, config_pb2.PlayerConfig(temperature=0.1))
    other = Player.find_or_create("NN", network, config_pb2.PlayerConfig(temperature=0))
    assert first == second
    assert first != other
    assert Player.find_or_create("RANDOM") == Player.find_or_create("RANDOM")
    assert Player.select().count() == 3


def test_duplicate_players_are_rejected_by_the_database(db):
    Player.create(type="RANDOM", config="")
    with pytest.raises(peewee.IntegrityError):
        Player.create(type="RANDOM", config="")  # NULL network: a plain UNIQUE would allow it


def test_player_settings_round_trip(db):
    network = add_network()
    player = Player.find_or_create("NN", network, config_pb2.PlayerConfig(temperature=0.25))
    player = Player.get_by_id(player.id)
    assert (player.type, player.network, player.config_proto) == (
        "NN",
        network,
        config_pb2.PlayerConfig(temperature=0.25),
    )


def test_random_player_takes_no_config(db):
    with pytest.raises(ValueError):
        Player.find_or_create("RANDOM", config=config_pb2.PlayerConfig(temperature=0.1))


def test_nn_player_needs_a_network(db):
    with pytest.raises(peewee.IntegrityError):
        Player.find_or_create("NN")


def test_unknown_network_is_rejected(db):
    with pytest.raises(peewee.IntegrityError):
        Player.create(type="NN", network=42, config="")


def test_check_constraints(db):
    with pytest.raises(peewee.IntegrityError):
        Player.create(type="HUMAN", config="")
    match = add_match(1)
    match.purpose = "FUN"
    with pytest.raises(peewee.IntegrityError):
        match.save()


def test_complete_match_and_summary(db):
    match = add_match(3)
    match.complete([make_game(True, 1, 40), make_game(False, -2, 60), make_game(True, 3, 80)])
    assert match.summary() == Summary(
        games=3,
        white_points=4,
        black_points=2,
        white_wins=2,
        black_wins=1,
        white_gammons=1,
        black_gammons=1,
        avg_turns=60.0,
    )
    match = Match.get_by_id(match.id)
    assert match.status == "COMPLETE"
    assert [game.final_score for game in match.game_protos()] == [1, -2, 3]
    assert [game.n_turns for game in match.games.order_by(Game.idx)] == [40, 60, 80]


def test_many_games_are_stored_in_batches(db):
    match = add_match(2500)
    match.complete(make_game(i % 2 == 0, 1, 1) for i in range(2500))
    assert match.games.count() == 2500


def test_wrong_game_count_stores_nothing(db):
    match = add_match(3)
    with pytest.raises(ValueError):
        match.complete([make_game(True, 1, 40)])
    assert Game.select().count() == 0
    assert Match.get_by_id(match.id).status == "RUNNING"


def test_failed_match_cannot_be_read(db):
    match = add_match(1)
    match.fail()
    assert Match.get_by_id(match.id).status == "FAILED"
    with pytest.raises(ValueError, match="not complete"):
        list(match.game_protos())


def test_summary_needs_games(db):
    with pytest.raises(KeyError):
        add_match(1).summary()


def test_queries_compose(db):
    training = add_match(2)
    training.complete([make_game(True, 2, 10), make_game(False, -1, 20)])
    evaluation = add_match(1)
    evaluation.purpose = "EVALUATION"
    evaluation.save()
    evaluation.complete([make_game(True, 3, 30)])
    gammons = Game.select().join(Match).where((Match.purpose == "TRAINING") & (fn.ABS(Game.final_score) >= 2))
    assert [(g.match.id, g.idx) for g in gammons] == [(training.id, 0)]


def test_game_files_round_trip(tmp_path):
    games = [make_game(True, 1, 5), make_game(False, -1, 7)]
    path = tmp_path / "games.pb"
    assert write_games(path, games) == 2
    assert list(read_games(path)) == games


def test_read_games_rejects_truncated_file(tmp_path):
    path = tmp_path / "games.pb"
    write_games(path, [make_game(True, 1, 5)])
    path.write_bytes(path.read_bytes()[:-1])
    with pytest.raises(ValueError, match="truncated"):
        list(read_games(path))
