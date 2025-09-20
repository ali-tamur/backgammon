from random import Random
from pathlib import Path

from src.dice_source import DiceSource
from src.game_manager import GameManager
from src.logger import LogWriter
from src.player import RandomPlayer

def find_project_root():
    current_file_path = Path(__file__).resolve()
    project_root = current_file_path
    while not (project_root / "requirements.txt").exists():
        project_root = project_root.parent
        if project_root == project_root.parent:  # To prevent infinite loop if file is not found
            raise FileNotFoundError("Could not find project root. Make sure requirements.txt exists.")
    return project_root

def main():
    project_root = find_project_root()
    script_dir = Path(__file__).parent
    my_schema = script_dir.parent / "schemas" / "log.avsc"
    log_file = script_dir.parent / "logs" / "mylog.avro"
    mylogger = LogWriter(log_file, my_schema)
    for i in range(19):
        seed_a = Random(17 + i)
        seed_b = Random(455 + i)
        alice = RandomPlayer("alice", seed_a)
        bob = RandomPlayer("bob", seed_b)
        dice_source = DiceSource(788 + i)
        game_manager = GameManager(alice, bob, dice_source, mylogger)
        score = game_manager.run_once()
        print(score)


if __name__ == "__main__":
    main()
