"""Training arrays for a set of matches, built by the C++ converter.

What an example is, and the meaning of each array, is defined in
cc/src/training_data.h. The arrays are a cache: they can always be rebuilt
from the games in the database. They are kept in data/cache/<key>/, where the
key covers the matches and the code version, so a stale cache is never reused.
With uncommitted changes there is no reliable code version, so nothing is
cached.
"""

import hashlib
import shutil
import subprocess
import tempfile
from collections.abc import Sequence
from pathlib import Path

import numpy as np

from bg.db import Match
from bg.game_log import write_games
from bg.match import git_commit, repo_root

CONVERTER = "//cc/src:training_data_main"
ARRAYS = ("boards", "labels", "points", "turn", "n_turns", "game")


def export_games(matches: Sequence[Match], path: Path) -> int:
    """Writes the matches' games, in the given match order, as a games file; returns the count."""
    return write_games(path, (data for match in matches for data in match.game_records()))


def convert(games_path: Path, out_dir: Path) -> None:
    result = subprocess.run(
        ["bazel", "run", "-c", "opt", CONVERTER, "--", f"--games={games_path}", f"--out={out_dir}"],
        cwd=repo_root(),
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"converter failed\n{result.stderr}")


def load(directory: Path) -> dict[str, np.ndarray]:
    return {name: np.load(directory / f"{name}.npy") for name in ARRAYS}


def build(matches: Sequence[Match], out_dir: Path) -> None:
    """Exports the matches' games and converts them into arrays in `out_dir`."""
    with tempfile.TemporaryDirectory(prefix="bg_export_") as tmp:
        games_path = Path(tmp) / "games.pb"
        export_games(matches, games_path)
        convert(games_path, out_dir)


def training_data(matches: Sequence[Match], cache_root: Path | None = None) -> dict[str, np.ndarray]:
    """The training arrays for the matches, from the cache when possible."""
    match_ids = [match.id for match in matches]
    root = repo_root()
    commit = git_commit(root, allow_dirty=True)
    if commit.endswith("-dirty"):
        with tempfile.TemporaryDirectory(prefix="bg_arrays_") as tmp:
            build(matches, Path(tmp))
            return load(Path(tmp))
    cache_root = cache_root or root / "data" / "cache"
    key = hashlib.sha256(f"matches={','.join(map(str, match_ids))};commit={commit}".encode()).hexdigest()[:16]
    directory = cache_root / key
    if not directory.exists():
        # Build next to the final place and rename, so an interrupted build
        # never leaves a half-written cache entry.
        cache_root.mkdir(parents=True, exist_ok=True)
        partial = Path(tempfile.mkdtemp(prefix=f"{key}.partial_", dir=cache_root))
        try:
            build(matches, partial)
            (partial / "inputs.txt").write_text(f"matches: {list(match_ids)}\ncommit: {commit}\n")
            partial.rename(directory)
        finally:
            shutil.rmtree(partial, ignore_errors=True)
    return load(directory)
