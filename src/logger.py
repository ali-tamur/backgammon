from typing import Optional
from pathlib import Path

import avro.schema
from avro.datafile import DataFileWriter, DataFileReader
from avro.io import DatumWriter, DatumReader


class GameKeeper:
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
            'game_id': self.game_id,
            'players': self.players,
            'state': self.states,
            'dice_roll': self.dice_rolls,
            'score': score
        }


class LogWriter:
    def __init__(self, file_path: Path, schema_path: Path):
        schema = avro.schema.parse(open(schema_path, "rt").read())
        self.writer = DataFileWriter(open(file_path, "ab"), DatumWriter(), schema)
        self.game_keeper: Optional[GameKeeper] = None

    def start_game(self, game_id, white, black):
        self.game_keeper = GameKeeper(game_id, white, black)

    def move(self, dice, state):
        self.game_keeper.add_move(dice, state)
        pass

    def end_game(self, score):
        record = self.game_keeper.end_game(score)
        try:
            self.writer.append(record)
        finally:
            self.writer.flush()

    def close(self):
        self.writer.close()


class LogReader:
    def __init__(self, schema_path: str):
        self.schema = avro.schema.parse(open(schema_path, "rt").read())
        self.records = []

    def read(self, file_path: str):
        try:
            with open(file_path, "rb") as f:
                with DataFileReader(f, DatumReader()) as reader:
                    for record in reader:
                        self.records.append(record)
        except IOError as e:
            print(f"Error reading file: {e}")
