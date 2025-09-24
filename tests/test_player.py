import pytest
import tensorflow as tf
import keras

from random import Random
from src.dice_source import Dice
from src.game_manager import GameManager
from src.player import NNPlayer, RandomPlayer
from src.loss_model import GameStateWeightedLoss
from src.main import find_project_root


from random import Random
from pathlib import Path
import src.main as m

class TestPlayer:
    @pytest.mark.integration
    def test_1(self):
        with tf.device('/CPU:0'):
            project_root = find_project_root()
            model_path = project_root / 'notebooks/my_model.keras'
            alice_model = tf.keras.models.load_model(model_path)
            r = Random(444)
            temperature = 2
            alice = NNPlayer('alice_1', alice_model, r, temperature)
            bob = RandomPlayer('b', r)
            dice_source = Dice(r)
            game_manager = GameManager(alice, bob, dice_source)
            r = [game_manager.run(1)]
            assert len(r) == 1
            print(r[0])
