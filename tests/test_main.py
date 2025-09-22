import pytest

from random import Random
from pathlib import Path
import src.main as m

class TestMain:
    def test_1(self):
        log_file =  Path('/app/data/game_log.avro')
        player_name = 'alice'
        result = m.read_data(log_file, player_name)
        assert len(result) > 0
