"""Game files: serialized Game protos (proto/game.proto), each prefixed with its
length as a little-endian uint32. play_games_main writes them and
training_data_main reads them (cc/src/logger.h)."""

import struct
from collections.abc import Iterable, Iterator
from pathlib import Path


def read_games(path: Path) -> Iterator[bytes]:
    """Yields the serialized games in the file, in order."""
    with path.open("rb") as f:
        while header := f.read(4):
            (size,) = struct.unpack("<I", header)
            data = f.read(size)
            if len(data) != size:
                raise ValueError(f"{path}: truncated game record")
            yield data


def write_games(path: Path, games: Iterable[bytes]) -> int:
    """Writes the serialized games, in order; returns their number."""
    count = 0
    with path.open("wb") as f:
        for data in games:
            f.write(struct.pack("<I", len(data)))
            f.write(data)
            count += 1
    return count
