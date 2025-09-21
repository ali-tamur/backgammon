from typing import Optional, List
from pathlib import Path

import avro.schema
from avro.datafile import DataFileWriter, DataFileReader
from avro.io import DatumWriter, DatumReader


class PlayerLog:
    def __init__(self, record, name):
        self.record = record
        self.players = self.record['players'].split('-')
        self.player_index = 0 if name == self.players[0] else 1

    @staticmethod
    def create(record, name):
        result = PlayerLog(record, name)
        return result if result.player_index is not None else None

    def game_states(self):
        if self.player_index is None:
            return None
        state_list = self.record['state']
        return state_list[self.player_index:len(state_list):2]

    def score(self):
        result_for_alice = self.record['score']
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
        self.game_keeper: Optional[GameRecordCreator] = None

    def start_game(self, game_id, white, black):
        self.game_keeper = GameRecordCreator(game_id, white, black)

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


def read_log(file_path: Path) -> List[dict]:
    records = []
    try:
       with open(file_path, "rb") as f:
           with DataFileReader(f, DatumReader()) as reader:
               for record in reader:
                   records.append(record)
    except IOError as e:
        print(f"Error reading file: {e}")
    return records

def read_for_player(file_path:Path, player_name):
    records = read_log(file_path)
    result = [PlayerLog.create(record, player_name) for record in records]
    return [r for r in result if r is not None]
