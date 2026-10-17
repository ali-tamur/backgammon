"""The project's SQLite database: networks, players, matches and games.

One Peewee model per table. Python is the database's only reader and writer;
the C++ binaries never see it. Settings that are still evolving live in
`config` columns as text-format protos (proto/config.proto); a field gets its
own column once queries need it.

Rows are created only by the operations that produce them: a network by
training, a player when a match needs it, a match and its games by playing.
The one exception is the random player, created with the database.

While the schema is settling, changing it means bumping SCHEMA_VERSION and
starting a new database (`python -m bg new-db`, which archives the old one).

    with open_db("data/backgammon.db"):
        random = Player.get(Player.type == "RANDOM")
        Match.select().where(Match.purpose == "TRAINING")
"""

import os
import shutil
import sqlite3
from collections.abc import Iterable, Iterator
from compression import zstd
from contextlib import contextmanager
from dataclasses import dataclass
from datetime import UTC, datetime
from itertools import batched
from pathlib import Path

from google.protobuf import text_format
from google.protobuf.message import Message
from peewee import (
    SQL,
    BlobField,
    BooleanField,
    Case,
    Check,
    CompositeKey,
    ForeignKeyField,
    IntegerField,
    Model,
    SqliteDatabase,
    TextField,
    fn,
)
from playhouse.sqlite_ext import AutoIncrementField

from proto import config_pb2, game_pb2

SCHEMA_VERSION = 1

# Peewee binds models to a database object when the classes are defined, long
# before the file is known: open_db() and create_db() point this placeholder at
# a file for the duration of a call. One database per process at a time.
database = SqliteDatabase(None, pragmas={"foreign_keys": 1})


def now() -> str:
    return datetime.now(UTC).isoformat(timespec="seconds")


def to_text(message: Message) -> str:
    """Canonical text format: the same message always gives the same string."""
    return text_format.MessageToString(message)


def one_of(column: str, values: tuple[str, ...]) -> Check:
    return Check(f"{column} IN ({', '.join(repr(v) for v in values)})")


class BaseModel(Model):
    class Meta:
        database = database


# AUTOINCREMENT ids are never reused, even after deleting the newest row:
# match ids, for one, also key the training-array cache.


class Network(BaseModel):
    id = AutoIncrementField()
    parent = ForeignKeyField("self", null=True, backref="children")
    created = TextField(default=now)
    git_commit = TextField()
    input_encoding = TextField()
    output_layout = TextField()
    onnx = BlobField()
    checkpoint = BlobField(null=True)  # PyTorch checkpoint, for training further
    config = TextField()  # NetworkConfig
    note = TextField(default="")

    @property
    def config_proto(self) -> config_pb2.NetworkConfig:
        return text_format.Parse(self.config, config_pb2.NetworkConfig())


PLAYER_TYPES = ("RANDOM", "NN")


class Player(BaseModel):
    """Immutable: a changed setting is a new player."""

    id = AutoIncrementField()
    type = TextField(constraints=[one_of("type", PLAYER_TYPES)])
    network = ForeignKeyField(Network, null=True, backref="players")
    config = TextField()  # PlayerConfig, canonical text format

    class Meta:
        constraints = [SQL("CHECK ((type = 'NN') = (network_id IS NOT NULL))")]

    @classmethod
    def find_or_create(
        cls, type: str, network: Network | None = None, config: config_pb2.PlayerConfig | None = None
    ) -> Player:
        text = to_text(config or config_pb2.PlayerConfig())
        if type == "RANDOM" and text:
            raise ValueError("random players take no config")
        player = cls.get_or_none(cls.type == type, cls.network == network, cls.config == text)
        return player or cls.create(type=type, network=network, config=text)

    @property
    def config_proto(self) -> config_pb2.PlayerConfig:
        return text_format.Parse(self.config, config_pb2.PlayerConfig())


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


PURPOSES = ("TRAINING", "EVALUATION")
FIRST_MOVES = ("ALTERNATE", "WHITE", "BLACK")
STATUSES = ("RUNNING", "COMPLETE", "FAILED")


class Match(BaseModel):
    id = AutoIncrementField()
    white = ForeignKeyField(Player, backref="white_matches")
    black = ForeignKeyField(Player, backref="black_matches")
    purpose = TextField(constraints=[one_of("purpose", PURPOSES)])
    first_move = TextField(constraints=[one_of("first_move", FIRST_MOVES)])
    seed = IntegerField()
    n_games = IntegerField()
    git_commit = TextField()  # with a '-dirty' suffix if the tree had changes
    created = TextField(default=now)
    status = TextField(default="RUNNING", constraints=[one_of("status", STATUSES)])
    config = TextField()  # MatchConfig
    note = TextField(default="")

    def complete(self, games: Iterable[bytes]) -> None:
        """Stores the games, in index order, and marks the match COMPLETE. All or nothing."""
        rows = (
            (self.id, idx, game.white_first, game.final_score, len(game.turns), data)
            for idx, data in enumerate(games)
            for game in [game_pb2.Game.FromString(data)]
        )
        fields = [Game.match, Game.idx, Game.white_first, Game.final_score, Game.n_turns, Game.data]
        with database.atomic():
            count = 0
            for batch in batched(rows, 1000):
                Game.insert_many(batch, fields=fields).execute()
                count += len(batch)
            if count != self.n_games:
                raise ValueError(f"match {self.id}: expected {self.n_games} games, got {count}")
            self.status = "COMPLETE"
            self.save()

    def fail(self) -> None:
        self.status = "FAILED"
        self.save()

    def game_records(self) -> Iterator[bytes]:
        """The serialized games, in index order."""
        if self.status != "COMPLETE":
            raise ValueError(f"match {self.id} is not complete")
        for (data,) in self.games.select(Game.data).order_by(Game.idx).tuples():
            yield data

    def game_protos(self) -> Iterator[game_pb2.Game]:
        for data in self.game_records():
            yield game_pb2.Game.FromString(data)

    def summary(self) -> Summary:
        def count(condition):
            return fn.SUM(Case(None, [(condition, 1)], 0))

        def points(condition, value):
            return fn.SUM(Case(None, [(condition, value)], 0))

        row = (
            Game.select(
                fn.COUNT(Game.idx),
                points(Game.final_score > 0, Game.final_score),
                points(Game.final_score < 0, 0 - Game.final_score),  # unary minus means DESC in Peewee
                count(Game.final_score > 0),
                count(Game.final_score < 0),
                count(Game.final_score >= 2),
                count(Game.final_score <= -2),
                fn.AVG(Game.n_turns),
            )
            .where(Game.match == self)
            .tuples()
            .get()
        )
        if row[0] == 0:
            raise KeyError(f"match {self.id} has no games")
        return Summary(*row)


class Game(BaseModel):
    match = ForeignKeyField(Match, backref="games", index=False)  # the primary key covers it
    idx = IntegerField()
    white_first = BooleanField()
    final_score = IntegerField()
    n_turns = IntegerField()
    data = BlobField()  # the Game proto (proto/game.proto)

    class Meta:
        primary_key = CompositeKey("match", "idx")

    @property
    def proto(self) -> game_pb2.Game:
        return game_pb2.Game.FromString(self.data)


MODELS = [Network, Player, Match, Game]


@contextmanager
def _bound(path: Path) -> Iterator[SqliteDatabase]:
    """Binds all models to the database at `path`, and unbinds them on leaving,
    even on an error; after that, queries fail instead of quietly reconnecting."""
    if database.database is not None:
        raise RuntimeError(f"a database is already open: {database.database}")
    database.init(str(path), pragmas={"foreign_keys": 1})
    try:
        database.connect()
        yield database
    finally:
        if not database.is_closed():
            database.close()
        # Peewee reconnects on the next query after close(); with no file
        # bound, it raises instead.
        database.init(None)


def create_db(path: Path | str) -> None:
    """Creates a database at `path`: the schema and the random player. Refuses an
    existing file. The file appears only once complete: it is built under a
    temporary name and renamed."""
    path = Path(path)
    if path.exists():
        raise FileExistsError(f"{path} already exists")
    partial = path.with_name(path.name + ".partial")
    partial.unlink(missing_ok=True)  # left by an interrupted create_db
    try:
        with _bound(partial), database.atomic():
            database.create_tables(MODELS)
            # Unique settings per player. Not a plain UNIQUE constraint: NULLs
            # are distinct there, so it would allow duplicate random players.
            database.execute_sql("CREATE UNIQUE INDEX player_settings ON player (type, IFNULL(network_id, 0), config)")
            # It has no settings, so there is exactly one; it is id 1, but look
            # it up by type.
            Player.create(type="RANDOM", config="")
            database.pragma("user_version", SCHEMA_VERSION)
        partial.rename(path)
    finally:
        partial.unlink(missing_ok=True)


@contextmanager
def open_db(path: Path | str) -> Iterator[SqliteDatabase]:
    """Binds all models to the existing database at `path` for the `with` block
    (create_db() makes one). Refuses a missing file: a mistyped path would
    otherwise be a new, empty database."""
    path = Path(path)
    if not path.is_file():
        raise FileNotFoundError(f"{path}: no such database (python -m bg new-db creates one)")
    with _bound(path):
        version = database.pragma("user_version")
        if version != SCHEMA_VERSION:
            raise RuntimeError(
                f"{path}: schema version {version}, this code expects {SCHEMA_VERSION}"
                " (python -m bg new-db starts a new database)"
            )
        yield database


def archive_db(path: Path | str, archive_dir: Path | str) -> Path:
    """Compresses the database at `path` into `archive_dir` (zstd, named by the
    time) and deletes it. Returns the archive's path. Refuses while another
    process has the database open."""
    path, archive_dir = Path(path), Path(archive_dir)
    if not path.is_file():
        raise FileNotFoundError(f"{path}: no such database")
    archive_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y-%m-%dT%H-%M-%S")
    archive = archive_dir / f"{path.stem}-{stamp}{path.suffix}.zst"
    n = 1
    while archive.exists():  # never overwrite: two archives in the same second
        archive = archive_dir / f"{path.stem}-{stamp}-{n}{path.suffix}.zst"
        n += 1
    partial = archive.with_name(archive.name + ".partial")
    # An exclusive lock keeps writers (and readers) out while the file is read,
    # and makes SQLite first roll back a transaction a crash left half done.
    conn = sqlite3.connect(path, timeout=0)
    try:
        conn.execute("BEGIN EXCLUSIVE")
        conn.execute("SELECT count(*) FROM sqlite_master")  # also: is it a database at all?
        with open(path, "rb") as src, zstd.open(partial, "wb") as dst:
            shutil.copyfileobj(src, dst, 1 << 20)
        os.replace(partial, archive)
    except sqlite3.OperationalError as e:
        if "locked" in str(e):
            raise RuntimeError(f"{path} is in use by another process") from e
        raise
    finally:
        conn.close()
        partial.unlink(missing_ok=True)
    path.unlink()
    return archive
