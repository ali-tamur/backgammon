import json
import os
from pathlib import Path

import fastavro
from fastavro import parse_schema


class PlayerLog:
    def __init__(self, record, name):
        self.record = record
        self.players = self.record["players"].split("-")
        self.player_index = 0 if name == self.players[0] else 1

    @staticmethod
    def create(record, name):
        result = PlayerLog(record, name)
        return result if result.player_index is not None else None

    def game_states(self):
        if self.player_index is None:
            return None
        state_list = self.record["state"]
        return state_list[self.player_index : len(state_list) : 2]

    def score(self):
        result_for_alice = self.record["score"]
        return result_for_alice if self.player_index == 0 else -result_for_alice


class GameRecordCreator:
    def __init__(self, game_id, white, black):
        self.game_id = game_id
        self.players = white + "-" + black
        self.dice_rolls = []
        self.states = []

    def add_move(self, dice, state):
        self.dice_rolls.append(dice)
        self.states.append(list(state.a))

    def end_game(self, score):
        return {
            "game_id": self.game_id,
            "players": self.players,
            "state": self.states,
            "dice_roll": self.dice_rolls,
            "score": score,
        }


class LogWriter:
    def __init__(self, file_path: Path, schema_path: Path):
        with open(schema_path) as f:
            schema_dict = json.load(f)
        self.schema = parse_schema(schema_dict)
        self.file_path = file_path
        self._check_new_file()
        self.game_keeper: GameRecordCreator | None = None
        self.file = None

    def _check_new_file(self):
        is_new_file = not os.path.exists(self.file_path) or os.path.getsize(self.file_path) == 0
        if is_new_file:
            with open(self.file_path, "wb") as out_file:
                fastavro.writer(out_file, self.schema, [])

    def __enter__(self):
        self.file = open(self.file_path, "a+b")

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.file:
            self.file.close()

    def start_game(self, game_id, white, black):
        self.game_keeper = GameRecordCreator(game_id, white, black)

    def move(self, dice, state):
        self.game_keeper.add_move(dice, state)

    def end_game(self, score):
        record = self.game_keeper.end_game(score)
        try:
            fastavro.writer(self.file, self.schema, [record])
        finally:
            self.file.flush()


class DummyLogWriter:
    def __init__(self):
        pass

    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def start_game(self, game_id, white, black):
        pass

    def move(self, dice, state):
        pass

    def end_game(self, score):
        pass


def read_log(file_path: Path) -> list[dict]:
    records = []
    try:
        with open(file_path, "rb") as f:
            avro_reader = fastavro.reader(f)
            for record in avro_reader:
                records.append(record)
    except OSError as e:
        print(f"Error reading file: {e}")
    return records


def read_for_player(file_path: Path, player_name):
    records = read_log(file_path)
    result = [PlayerLog.create(record, player_name) for record in records]
    return [r for r in result if r is not None]
