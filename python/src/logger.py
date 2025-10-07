import pickle
import sqlite3
from collections.abc import Iterator
from dataclasses import astuple, dataclass
from pathlib import Path
from typing import Optional

from python.src.state import GAME_START


@dataclass
class Record:
    id: int | None
    game_id: int
    player: str
    move_number: int
    num_moves: int
    prev_board: bytes
    dice: int
    next_board: bytes
    score: int


class DummyLogWriter:
    def __init__(self):
        pass

    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def start_game(self, white, black):
        pass

    def move(self, dice, state):
        pass

    def end_game(self, score):
        pass


class LogBuffer:
    def __init__(self, batch_size: int, db_path: str):
        self.batch_size = batch_size
        self.conn = sqlite3.connect(str(db_path))
        self._create_table()
        self.buffer: list[Record] = []

    def _create_table(self):
        self.conn.execute("""
            CREATE TABLE IF NOT EXISTS game_moves (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                game_id INTEGER NOT NULL,
                player TEXT NOT NULL,
                move_number INTEGER NOT NULL,
                num_moves INTEGER NOT NULL,
                prev_board BLOB NOT NULL,
                dice INTEGER NOT NULL,
                next_board BLOB NOT NULL,
                score INTEGER NOT NULL
            )
        """)
        self.conn.execute(""" CREATE INDEX IF NOT EXISTS idx_game_id ON game_moves(game_id) """)
        self.conn.commit()

    def append(self, record: Record):
        self.buffer.append(record)

    def maybe_flush(self):
        if len(self.buffer) >= self.batch_size:
            self.flush()

    def flush(self):
        self.conn.executemany(
            "INSERT INTO game_moves VALUES (?,?,?,?,?,?,?,?,?)", (astuple(move) for move in self.buffer)
        )
        self.conn.commit()
        self.buffer.clear()

    def close(self):
        self.flush()
        self.conn.close()


class GameRecorder:
    def __init__(self, game_id: int, log_buffer: LogBuffer, white, black):
        self.game_id = game_id
        self.log_buffer = log_buffer
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
        r = Record(
            id=None,
            game_id=self.game_id,
            player=player,
            move_number=i,
            num_moves=num_moves,
            prev_board=prev_board,
            dice=dice,
            next_board=next_board,
            score=score,
        )
        self.log_buffer.append(r)


class LogWriter:
    def __init__(self, db_path: Path | str):
        self.db_path = Path(db_path)
        Path(db_path).parent.mkdir(exist_ok=True)
        self.log_buffer: Optional[LogBuffer] = None
        self.game_recorder: Optional[GameRecorder] = None

    def __enter__(self):
        self.log_buffer = LogBuffer(5000, str(self.db_path))
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.log_buffer:
            self.log_buffer.close()
        return False

    def start_game(self, game_id: int, white: str, black: str):
        assert self.log_buffer
        self.game_recorder = GameRecorder(game_id, self.log_buffer, white, black)

    def move(self, dice: int, state: list[int]):
        assert self.game_recorder
        self.game_recorder.add_move(dice, state)

    def end_game(self, score):
        assert self.game_recorder
        assert self.log_buffer
        self.game_recorder.end_game(score)


class LogReader:
    def __init__(self, db_path: Path | str):
        self.db_path = Path(db_path)

    def get_player_moves(self, player_name: str) -> Iterator[tuple[list[int], int, int, int] | None]:
        conn = sqlite3.connect(self.db_path)
        try:
            cursor = conn.execute("""SELECT * FROM game_moves WHERE player = ?""", (player_name,))
            for row in cursor:
                record = Record(*row)
                next_board: list[int] = pickle.loads(record.next_board)
                move_number = record.move_number
                num_moves = record.num_moves
                score = record.score
                yield (next_board, move_number, num_moves, score)
            yield None
        finally:
            conn.close()
