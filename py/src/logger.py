import os
import pickle
import sqlite3
from pathlib import Path
from typing import Optional

from py.src.state import GAME_START


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


class GameRecorder:
    def __init__(self, log_buffer, game_id, white, black):
        self.log_buffer = log_buffer
        self.game_id = game_id
        self.white = white
        self.black = black
        self.states = [GAME_START]
        self.dices = []

    def add_move(self, dice, state):
        self.dices.append(dice)
        self.states.append(state)

    def end_game(self, score):
        for i in range(1, len(self.states), 2):
            self._append(i, self.white, score)
        for i in range(2, len(self.states), 2):
            self._append(i, self.black, score)
        self.log_buffer.maybe_flush()

    def _append(self, i, player, score):
        num_moves = len(self.states) - 1
        prev_board = pickle.dumps(self.states[i - 1])
        next_board = pickle.dumps(self.states[i])
        dice = self.dices[i - 1]
        record = (self.game_id, self.white, i, num_moves, prev_board, dice, next_board, score)
        self.log_buffer.append(record)


class LogBuffer:
    def __init__(self, batch_size, db_path):
        self.batch_size = batch_size
        self.conn = sqlite3.connect(db_path)
        self._create_table()
        self.buffer = []

    def _create_table(self):
        self.conn.execute("""
            CREATE TABLE IF NOT EXISTS game_moves (
                game_id TEXT NOT NULL,
                player TEXT NOT NULL,
                move_number INTEGER NOT NULL,
                total_moves INTEGER,
                prev_board BLOB NOT NULL,
                dice INTEGER NOT NULL,
                next_board BLOB NOT NULL,
                score INTEGER
            )
        """)
        self.conn.commit()

    def append(self, record):
        self.buffer.append(record)

    def maybe_flush(self):
        if len(self.buffer) >= self.batch_size:
            self.flush()

    def flush(self):
        self.conn.executemany("INSERT INTO game_moves VALUES (?,?,?,?,?,?,?,?)", self.buffer)
        self.conn.commit()
        self.buffer.clear()

    def close(self):
        self.flush()
        self.conn.close()


class LogWriter:
    def __init__(self, db_path: Path):
        self.db_path = db_path
        Path(db_path).parent.mkdir(exist_ok=True)
        self.log_buffer: Optional[LogBuffer] = None
        self.game_recorder: Optional[GameRecorder] = None

    def __enter__(self):
        self.log_buffer = LogBuffer(5000, self.db_path)
        return self

    def __exit__(self):
        if self.log_buffer:
            self.log_buffer.close()
        return False

    def start_game(self, game_id, white, black):
        assert self.log_buffer
        self.game_recorder = GameRecorder(self.log_buffer.buffer, game_id, white, black)

    def move(self, dice, state):
        assert self.game_recorder
        self.game_recorder.add_move(dice, state)

    def end_game(self, score):
        assert self.game_recorder
        assert self.log_buffer
        self.game_recorder.end_game(score)


class LogReader:
    def __init__(self, db_path: str):
        self.db_path = db_path

    def get_player_moves(self, player_name: str):
        """
        Yields (next_board, move_number, total_moves, score) for player_name.
        """
        conn = sqlite3.connect(self.db_path)
        cursor = conn.execute(
            """SELECT next_board, move_number, total_moves, score 
               FROM game_moves 
               WHERE player = ?""",
            (player_name,),
        )

        for row in cursor:
            next_board = pickle.loads(row[0])
            move_number = row[1]
            total_moves = row[2]
            score = row[3]
            yield (next_board, move_number, total_moves, score)

        conn.close()
