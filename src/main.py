import argparse
import os
from pathlib import Path
from random import Random

from dotenv import load_dotenv

from src.dice_source import DiceSource
from src.game_manager import GameManager
from src.logger import LogWriter
from src.move_generator import WhiteGenerator, WhiteGeneratorFactory, BlackGeneratorFactory
from src.player import RandomPlayer
from src.training_data import prepare_training_data

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
    parser = argparse.ArgumentParser(description="Run a backgammon AI simulation.")

    subparsers = parser.add_subparsers(dest="command")
    play_games_parser = subparsers.add_parser(
        "play_games", help="Run a simulation for a specified number of games.")
    play_games_parser.add_argument(
        "--count",
        type=int,
        default=0,
        help="The number of games to play."
    )
    play_games_parser.add_argument(
        "--debug",
        action='store_true',
        help="Will print the game while it is played if enabled"
    )

    read_data_parser = subparsers.add_parser(
        "read_data", help="Read a dataset of training data.")
    read_data_parser.add_argument(
        "--player_name",
        type=str,
        required=True,
        help="The player name whose games will be loaded"
    )

    args = parser.parse_args()
    return args


def read_data(log_file: Path, player_name: str):
    return prepare_training_data(log_file, player_name)


def play_games(alice, bob, dice_source, n_games, log_writer, will_debug):
    game_manager = GameManager(alice, bob, dice_source, log_writer, will_debug)
    game_manager.run(n_games)


def get_log_file() -> Path:
    load_dotenv()
    log_file_path = os.getenv("LOG_FILE_PATH") or DEFAULT_LOG_PATH
    log_file = Path(log_file_path)
    return log_file


def get_schema() -> Path:
    load_dotenv()
    log_schema_path = os.getenv("LOG_SCHEMA_PATH") or DEFAULT_LOG_SCHEMA_PATH
    project_root = find_project_root()
    schema = project_root / log_schema_path
    return schema


def create_log_writer():
    schema = get_schema()
    log_file = get_log_file()
    return LogWriter(log_file, schema)


def main():
    random_seed = Random(31415)
    args = parse_args()
    if args.command == "play_games":
        n_games = args.count or 1000
        alice = RandomPlayer('alice', random_seed, WhiteGeneratorFactory)
        bob = RandomPlayer('bob', random_seed, BlackGeneratorFactory)
        log_writer = create_log_writer()
        play_games(alice, bob, DiceSource(random_seed), n_games, log_writer, args.debug)
    elif args.command == "read_data":
        player = args.player_name
        read_data(get_log_file(), player)


if __name__ == "__main__":
    main()
