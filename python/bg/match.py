"""Plays a match with the C++ engine and stores it in the database.

The engine (//cc/src:play_games_main) knows nothing about the database: this
module gives it the players' settings, lets it write its games to a temporary
file, and imports that file in a single transaction.
"""

import secrets
import shlex
import subprocess
import tempfile
from pathlib import Path

from bg.db import Db, read_games
from proto import config_pb2

ENGINE = "//cc/src:play_games_main"
FIRST_MOVE_FLAGS = {"ALTERNATE": "--take_turns", "WHITE": "--white_first", "BLACK": "--black_first"}


def repo_root() -> Path:
    out = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True, check=True)
    return Path(out.stdout.strip())


def git_commit(root: Path, allow_dirty: bool) -> str:
    """HEAD's commit; refuses uncommitted changes to tracked files unless allowed."""
    commit = subprocess.run(
        ["git", "rev-parse", "HEAD"], cwd=root, capture_output=True, text=True, check=True
    ).stdout.strip()
    dirty = subprocess.run(
        ["git", "status", "--porcelain", "--untracked-files=no"], cwd=root, capture_output=True, text=True, check=True
    ).stdout.strip()
    if not dirty:
        return commit
    if not allow_dirty:
        raise RuntimeError("uncommitted changes: commit them, or pass allow_dirty=True to record a '-dirty' commit")
    return commit + "-dirty"


def player_flags(db: Db, player_id: int, color: str, workdir: Path) -> list[str]:
    """Engine flags for one side; `color` is 'w' or 'b'."""
    type, network_id, config = db.player_settings(player_id)
    if type == "RANDOM":
        return [f"--{color}_model=random"]
    onnx_path = workdir / f"{color}_network_{network_id}.onnx"
    onnx_path.write_bytes(db.network_onnx(network_id))
    return [f"--{color}_model={onnx_path}", f"--{color}_temperature={config.temperature}"]


def run_match(
    db: Db,
    white_id: int,
    black_id: int,
    n_games: int,
    *,
    purpose: str,
    first_move: str = "ALTERNATE",
    seed: int | None = None,
    allow_dirty: bool = False,
    note: str = "",
) -> int:
    """Plays `n_games` games between two players and returns the new match's id."""
    root = repo_root()
    commit = git_commit(root, allow_dirty)
    # SQLite integers are signed 64-bit.
    seed = secrets.randbits(63) if seed is None else seed
    with tempfile.TemporaryDirectory(prefix="bg_match_") as tmp:
        workdir = Path(tmp)
        games_path = workdir / "games.pb"
        command = [
            "bazel",
            "run",
            "-c",
            "opt",
            ENGINE,
            "--",
            *player_flags(db, white_id, "w", workdir),
            *player_flags(db, black_id, "b", workdir),
            f"--n_games={n_games}",
            f"--seed={seed}",
            FIRST_MOVE_FLAGS[first_move],
            "--print_score=false",
            f"--log_path={games_path}",
        ]
        config = config_pb2.MatchConfig(command=shlex.join(command))
        match_id = db.create_match(
            white_id,
            black_id,
            purpose=purpose,
            first_move=first_move,
            seed=seed,
            n_games=n_games,
            git_commit=commit,
            config=config,
            note=note,
        )
        try:
            # Bazel's progress output goes to stderr; show it only if something fails.
            result = subprocess.run(command, cwd=root, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
            if result.returncode != 0:
                raise RuntimeError(f"match {match_id}: engine failed\n{result.stderr}")
            db.complete_match(match_id, read_games(games_path))
        except BaseException:
            db.fail_match(match_id)
            raise
    return match_id
