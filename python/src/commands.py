import statistics
import time
from pathlib import Path
from random import Random

from tensorflow.keras.models import load_model  # type: ignore

from python.src.cc_generator import generate_black_moves, generate_white_moves
from python.src.dice_source import Dice
from python.src.game_manager import GameManager
from python.src.logger import DummyLogWriter, LogWriter
from python.src.player import NNPlayer, RandomPlayer
from python.src.training_data import prepare_training_data


# TODO: provide temperature parameter to NN models (currently hard coded as 0.1).
# TODO: provide number of runs for benchmark (currently hard coded as 5)
def _play(
    white_name: str,
    white_model_path: Path | None,
    black_name: str,
    black_model_path: Path | None,
    n_games: int,
    db_path: Path | None,
    seed: int | None,
    is_benchmark: bool,
    is_interactive: bool,
):
    temperature = 0.1
    r = Random(seed)
    dice = Dice(r)
    if white_name == "random":
        alice = RandomPlayer("alice", r, generate_white_moves)
    else:
        assert white_model_path
        white_model = load_model(white_model_path)
        alice = NNPlayer(white_name, white_model, r, temperature, generate_white_moves)

    if black_name == "random":
        bob = RandomPlayer("bob", r, generate_black_moves)
    else:
        assert black_model_path
        black_model = load_model(black_model_path)
        bob = NNPlayer(black_name, black_model, r, temperature, generate_black_moves)
    if db_path is None:
        log_writer = DummyLogWriter()
    else:
        log_writer = LogWriter(db_path)
    game_manager = GameManager(alice, bob, dice, log_writer, is_interactive)
    if is_benchmark:
        times = []
        num_runs = 5
        for i in range(num_runs):
            start = time.perf_counter()
            game_manager.run(n_games)
            elapsed = time.perf_counter() - start
            times.append(elapsed)
        print(f"\nMean: {statistics.mean(times):.3f}s")
        print(f"\nStdev: {statistics.stdev(times):.3f}s")
    else:
        game_manager.run(n_games)


def c_play(white, white_model, black, black_model, count, db_path):
    return _play(white, white_model, black, black_model, count, db_path, None, False, False)


def c_benchmark(white, white_model, black, black_model, count, seed):
    return _play(white, white_model, black, black_model, count, None, seed, True, False)


def c_interactive(white, white_model, black, black_model):
    return _play(white, white_model, black, black_model, 1, None, None, False, True)


def c_load_data(player_name: str, alt_player: str | None, limit: int, db_path: str):
    return prepare_training_data(Path(db_path), player_name, alt_player, limit)
