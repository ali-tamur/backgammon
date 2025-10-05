from pathlib import Path

import pytest
import src.main as m
import tensorflow as tf


class TestMain:
    @pytest.mark.integration
    def test_1(self):
        with tf.device("/CPU:0"):
            log_file = Path("/app/data/game_log.avro")
            player_name = "alice"
            result = m.read_data(log_file, player_name)
            assert len(result) > 0
