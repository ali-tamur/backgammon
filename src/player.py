import random
from abc import ABC, abstractmethod
from typing import Callable

import keras
import tensorflow as tf

from src.dice import Dice
from src.move_generator import MoveGenerator
from src.state import GameState
from src.training_data import state_to_tensor


class Player(ABC):
    def __init__(self, name, move_generator: Callable[[GameState], MoveGenerator]):
        self.name = name
        self.move_generator = move_generator
        pass

    @abstractmethod
    def move(self, state, dice):
        pass


class RandomPlayer(Player):
    def __init__(self, name, r: random.Random):
        super().__init__(name)
        self.r = r

    def move(self, state: GameState, dice: Dice) -> GameState:
        moves = list(self.move_generator(state).generate_moves(dice))
        return self.r.choice(moves)


class NNPlayer(Player):
    def __init__(self, name, model: keras.Model, r: random.Random, temperature):
        super().__init__(name)
        self.r = r
        self.model = model
        self.temperature = temperature

    def move(self, state: GameState, dice: Dice) -> GameState:
        moves = list(self.move_generator(state).generate_moves(dice))
        tensors = tf.stack([state_to_tensor(st.a) for st in moves])
        raw_scores = self.model.predict(tensors, verbose=0)
        pure_weights = tf.squeeze(raw_scores, [-1]) / self.temperature
        exp_weights = tf.math.exp(pure_weights)
        exp_weights_np = exp_weights.numpy()
        return self.r.choices(moves, weights=exp_weights_np)[0]
