"""The project's SQLite database: networks, players, matches and games.

Python is the database's only reader and writer; the C++ binaries never see it.
Settings that are still evolving live in `config` columns as text-format protos
(proto/config.proto); a field gets its own column once queries need it.

While the schema is settling, changing it means bumping SCHEMA_VERSION and
regenerating the database from scratch.
"""

import sqlite3
import struct
from collections.abc import Iterable, Iterator
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path

from google.protobuf import text_format
from google.protobuf.message import Message

from proto import config_pb2, game_pb2

SCHEMA_VERSION = 1

_SCHEMA = """
CREATE TABLE network (
  id             INTEGER PRIMARY KEY,
  parent_id      INTEGER REFERENCES network(id),
  created        TEXT NOT NULL,
  git_commit     TEXT NOT NULL,
  input_encoding TEXT NOT NULL,
  output_layout  TEXT NOT NULL,
  onnx           BLOB NOT NULL,
  checkpoint     BLOB,             -- PyTorch checkpoint, for training further
  config         TEXT NOT NULL,    -- NetworkConfig
  note           TEXT NOT NULL DEFAULT ''
);

-- Immutable: a changed setting is a new player.
CREATE TABLE player (
  id         INTEGER PRIMARY KEY,
  type       TEXT NOT NULL CHECK (type IN ('RANDOM', 'NN')),
  network_id INTEGER REFERENCES network(id),
  config     TEXT NOT NULL,        -- PlayerConfig, canonical text format
  CHECK ((type = 'NN') = (network_id IS NOT NULL)),
  UNIQUE (type, network_id, config)
);

CREATE TABLE match (
  id         INTEGER PRIMARY KEY,
  white_id   INTEGER NOT NULL REFERENCES player(id),
  black_id   INTEGER NOT NULL REFERENCES player(id),
  purpose    TEXT NOT NULL CHECK (purpose IN ('TRAINING', 'EVALUATION')),
  first_move TEXT NOT NULL CHECK (first_move IN ('ALTERNATE', 'WHITE', 'BLACK')),
  seed       INTEGER NOT NULL,
  n_games    INTEGER NOT NULL,
  git_commit TEXT NOT NULL,         -- with a '-dirty' suffix if the tree had changes
  created    TEXT NOT NULL,
  status     TEXT NOT NULL CHECK (status IN ('RUNNING', 'COMPLETE', 'FAILED')),
  config     TEXT NOT NULL,         -- MatchConfig
  note       TEXT NOT NULL DEFAULT ''
);

CREATE TABLE game (
  match_id    INTEGER NOT NULL REFERENCES match(id),
  idx         INTEGER NOT NULL,
  white_first INTEGER NOT NULL,
  final_score INTEGER NOT NULL,
  n_turns     INTEGER NOT NULL,
  data        BLOB NOT NULL,        -- the Game proto (proto/game.proto)
  PRIMARY KEY (match_id, idx)
);

CREATE VIEW match_summary AS
SELECT match_id,
       COUNT(*)                                                AS games,
       SUM(CASE WHEN final_score > 0 THEN final_score ELSE 0 END)  AS white_points,
       SUM(CASE WHEN final_score < 0 THEN -final_score ELSE 0 END) AS black_points,
       SUM(final_score > 0)                                    AS white_wins,
       SUM(final_score < 0)                                    AS black_wins,
       SUM(final_score >= 2)                                   AS white_gammons,
       SUM(final_score <= -2)                                  AS black_gammons,
       AVG(n_turns)                                            AS avg_turns
FROM game
GROUP BY match_id;
"""


@dataclass(frozen=True)
class Summary:
    games: int
    white_points: int
    black_points: int
    white_wins: int
    black_wins: int
    white_gammons: int
    black_gammons: int
    avg_turns: float


def now() -> str:
    return datetime.now(UTC).isoformat(timespec="seconds")


def to_text(message: Message) -> str:
    """Canonical text format: the same message always gives the same string."""
    return text_format.MessageToString(message)


def read_games(path: Path) -> Iterator[bytes]:
    """Yields the serialized games of a play_games_main log, in order."""
    with path.open("rb") as f:
        while header := f.read(4):
            (size,) = struct.unpack("<I", header)
            data = f.read(size)
            if len(data) != size:
                raise ValueError(f"{path}: truncated game record")
            yield data


class Db:
    def __init__(self, conn: sqlite3.Connection):
        self.conn = conn

    @classmethod
    def open(cls, path: Path | str) -> "Db":
        """Opens the database, creating the schema in a new or empty file."""
        conn = sqlite3.connect(path)
        conn.execute("PRAGMA foreign_keys = ON")
        version = conn.execute("PRAGMA user_version").fetchone()[0]
        if version == 0:
            with conn:
                conn.executescript(_SCHEMA)
                conn.execute(f"PRAGMA user_version = {SCHEMA_VERSION}")
        elif version != SCHEMA_VERSION:
            raise RuntimeError(
                f"{path}: schema version {version}, this code expects {SCHEMA_VERSION}; regenerate the database"
            )
        return cls(conn)

    def close(self) -> None:
        self.conn.close()

    # --- networks

    def add_network(
        self,
        onnx: bytes,
        config: config_pb2.NetworkConfig,
        *,
        input_encoding: str,
        output_layout: str,
        git_commit: str,
        checkpoint: bytes | None = None,
        parent_id: int | None = None,
        note: str = "",
    ) -> int:
        with self.conn:
            cur = self.conn.execute(
                "INSERT INTO network (parent_id, created, git_commit, input_encoding, output_layout,"
                " onnx, checkpoint, config, note) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
                (parent_id, now(), git_commit, input_encoding, output_layout, onnx, checkpoint, to_text(config), note),
            )
        return cur.lastrowid

    def network_onnx(self, network_id: int) -> bytes:
        row = self.conn.execute("SELECT onnx FROM network WHERE id = ?", (network_id,)).fetchone()
        if row is None:
            raise KeyError(f"no network {network_id}")
        return row[0]

    # --- players

    def player(self, type: str, network_id: int | None = None, config: config_pb2.PlayerConfig | None = None) -> int:
        """Returns the id of the player with these settings, creating it if needed."""
        text = to_text(config or config_pb2.PlayerConfig())
        if type == "RANDOM" and text:
            raise ValueError("random players take no config")
        row = self.conn.execute(
            "SELECT id FROM player WHERE type = ? AND network_id IS ? AND config = ?", (type, network_id, text)
        ).fetchone()
        if row is not None:
            return row[0]
        with self.conn:
            cur = self.conn.execute(
                "INSERT INTO player (type, network_id, config) VALUES (?, ?, ?)", (type, network_id, text)
            )
        return cur.lastrowid

    def player_settings(self, player_id: int) -> tuple[str, int | None, config_pb2.PlayerConfig]:
        row = self.conn.execute("SELECT type, network_id, config FROM player WHERE id = ?", (player_id,)).fetchone()
        if row is None:
            raise KeyError(f"no player {player_id}")
        type, network_id, text = row
        return type, network_id, text_format.Parse(text, config_pb2.PlayerConfig())

    # --- matches

    def create_match(
        self,
        white_id: int,
        black_id: int,
        *,
        purpose: str,
        first_move: str,
        seed: int,
        n_games: int,
        git_commit: str,
        config: config_pb2.MatchConfig,
        note: str = "",
    ) -> int:
        """Records a match about to be played, with status RUNNING."""
        with self.conn:
            cur = self.conn.execute(
                "INSERT INTO match (white_id, black_id, purpose, first_move, seed, n_games, git_commit,"
                " created, status, config, note) VALUES (?, ?, ?, ?, ?, ?, ?, ?, 'RUNNING', ?, ?)",
                (white_id, black_id, purpose, first_move, seed, n_games, git_commit, now(), to_text(config), note),
            )
        return cur.lastrowid

    def complete_match(self, match_id: int, games: Iterable[bytes]) -> None:
        """Stores the match's games, in index order, and marks it COMPLETE. All or nothing."""
        n_games = self.conn.execute("SELECT n_games FROM match WHERE id = ?", (match_id,)).fetchone()[0]
        with self.conn:
            count = 0
            for idx, data in enumerate(games):
                game = game_pb2.Game.FromString(data)
                self.conn.execute(
                    "INSERT INTO game (match_id, idx, white_first, final_score, n_turns, data)"
                    " VALUES (?, ?, ?, ?, ?, ?)",
                    (match_id, idx, game.white_first, game.final_score, len(game.turns), data),
                )
                count += 1
            if count != n_games:
                raise ValueError(f"match {match_id}: expected {n_games} games, got {count}")
            self.conn.execute("UPDATE match SET status = 'COMPLETE' WHERE id = ?", (match_id,))

    def fail_match(self, match_id: int) -> None:
        with self.conn:
            self.conn.execute("UPDATE match SET status = 'FAILED' WHERE id = ?", (match_id,))

    def summary(self, match_id: int) -> Summary:
        row = self.conn.execute(
            "SELECT games, white_points, black_points, white_wins, black_wins, white_gammons,"
            " black_gammons, avg_turns FROM match_summary WHERE match_id = ?",
            (match_id,),
        ).fetchone()
        if row is None:
            raise KeyError(f"match {match_id} has no games")
        return Summary(*row)

    def game_records(self, match_id: int) -> Iterator[bytes]:
        """The match's serialized games, in index order."""
        status = self.conn.execute("SELECT status FROM match WHERE id = ?", (match_id,)).fetchone()
        if status is None or status[0] != "COMPLETE":
            raise ValueError(f"match {match_id} is not complete")
        rows = self.conn.execute("SELECT data FROM game WHERE match_id = ? ORDER BY idx", (match_id,))
        for (data,) in rows:
            yield data

    def games(self, match_id: int) -> Iterator[game_pb2.Game]:
        for data in self.game_records(match_id):
            yield game_pb2.Game.FromString(data)
