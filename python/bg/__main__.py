"""Command line for the operations: python -m bg <command> (--help lists them).

Each command only parses its arguments and calls library functions; exploring
the data is done in Python, with the models in bg.db.
"""

import argparse
import shutil
import sys
from collections.abc import Sequence
from pathlib import Path

from bg.db import archive_db, create_db
from bg.match import repo_root


def default_db() -> Path:
    return repo_root() / "data" / "backgammon.db"


def new_db(db: Path) -> None:
    """Archives the current database (if any) to archive/ next to it, deletes the
    training data cache next to it (cache/: made from the old games) and creates
    an empty database."""
    if db.exists():
        archive = archive_db(db, db.parent / "archive")
        print(f"archived {db} to {archive}")
        print(f"  to undo: zstd -d {archive} -o {db} (after moving the new one away)")
        print(f"           and delete {db.parent / 'cache'}")
    shutil.rmtree(db.parent / "cache", ignore_errors=True)
    db.parent.mkdir(parents=True, exist_ok=True)
    create_db(db)
    print(f"created {db}")


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="python -m bg", description=__doc__.splitlines()[0])
    parser.add_argument("--db", type=Path, help="the database (default: data/backgammon.db in the repository)")
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("new-db", help="start a new database, archiving the current one", description=new_db.__doc__)
    args = parser.parse_args(argv)

    db = args.db or default_db()
    match args.command:
        case "new-db":
            new_db(db)
    return 0


if __name__ == "__main__":
    sys.exit(main())
