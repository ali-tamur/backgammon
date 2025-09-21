from random import Random
from pathlib import Path
import os
import argparse
import time

from jupyter_server import DEFAULT_EVENTS_SCHEMA_PATH

from src.dice_source import DiceSource
from src.game_manager import GameManager
from src.logger import LogWriter, DummyLogWriter
from src.player import RandomPlayer
from dotenv import load_dotenv


DEFAULT_LOG_SCHEMA_PATH = "schemas/log.avsc"
DEFAULT_LOG_PATH = "/app/data/game_log.avro"

def find_project_root():
    current_file_path = Path(__file__).resolve()
    project_root = current_file_path
    while not (project_root / "requirements.txt").exists():
        project_root = project_root.parent
        if project_root == project_root.parent:  # To prevent infinite loop if file is not found
            raise FileNotFoundError("Could not find project root. Make sure requirements.txt exists.")
    return project_root

def parse_args():
    parser = argparse.ArgumentParser(
        description="Run a backgammon AI simulation.")

    subparsers = parser.add_subparsers(dest="command")
    play_games_parser = subparsers.add_parser(
        "play_games", help="Run a simulation for a specified number of games.")

    play_games_parser.add_argument(
        "--count",
        type=int,
        default=1,
        help="The number of games to play."
    )

    args = parser.parse_args()
    return args

def main():
    #args = parse_args()
#    n_games = args.count
    n_games = 200
    load_dotenv()
    log_schema_path = os.getenv("LOG_SCHEMA_PATH") or DEFAULT_LOG_SCHEMA_PATH
    log_file_path = os.getenv("LOG_FILE_PATH") or DEFAULT_LOG_PATH

    project_root = find_project_root()
    my_schema = project_root / log_schema_path
    log_file = Path(log_file_path)
    mylogger = LogWriter(log_file, my_schema)
    #mylogger = DummyLogWriter()
    random_seed = Random(time.time())
    alice = RandomPlayer("alice", random_seed)
    bob = RandomPlayer("bob", random_seed)
    dice_source = DiceSource(random_seed)
    game_manager = GameManager(alice, bob, dice_source, mylogger)
    game_manager.run(n_games)


if __name__ == "__main__":
    main()
