import os
import tempfile
import pytest
from pathlib import Path

from python.src.logger import LogReader, LogWriter


class TestLogger:
    def test_basic(self):
        test_dir = Path("/tmp/backgammon_test")
        test_dir.mkdir(exist_ok=True)

        tmp = tempfile.NamedTemporaryFile(suffix=".db", delete=False, dir=test_dir)
        tmp_path = tmp.name
        tmp.close()

        try:
            with LogWriter(tmp_path) as writer:
                writer.start_game("test_1", "alice", "bob")
                writer.move(35, [1, 0, 0] + [0] * 25)
                writer.end_game(1)

            reader = LogReader(tmp_path)
            moves = list(reader.get_player_moves("alice"))
            assert len(moves) == 1
        finally:
            if os.path.exists(tmp_path):
                os.remove(tmp_path)
