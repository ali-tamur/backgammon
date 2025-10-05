import random
from abc import ABC, abstractmethod
from collections.abc import Callable

import keras
import tensorflow as tf

from py.src.state import GameState
from py.src.training_data import state_to_tensor


class Player(ABC):
    def __init__(self, name, move_generator: Callable[[GameState, int, int], list[list[int]]]):
        self.name = name
        self.move_generator = move_generator
        pass

    @abstractmethod
    def move(self, state, d1, d2) -> GameState:
        pass


class RandomPlayer(Player):
    def __init__(self, name, r: random.Random, move_generator):
        super().__init__(name, move_generator)
        self.r = r

    def move(self, state: GameState, d1: int, d2: int) -> GameState:
        moves = self.move_generator(state.a, d1, d2)
        chosen_move = self.r.choice(moves)
        return GameState(chosen_move)


class NNPlayer(Player):
    def __init__(self, name, model: keras.Model, r: random.Random, temperature, move_generator):
        super().__init__(name, move_generator)
        self.r = r
        self.model = model
        self.temperature = temperature

    def move(self, state: GameState, d1: int, d2: int) -> GameState:
        moves = self.move_generator(state.a, d1, d2)
        tensors = tf.stack([state_to_tensor(st) for st in moves])
        raw_scores = self.model.predict(tensors, verbose=0)
        pure_weights = tf.squeeze(raw_scores, [-1]) / self.temperature
        exp_weights = tf.math.exp(pure_weights)
        exp_weights_np = exp_weights.numpy()
        chosen_move = self.r.choices(moves, weights=exp_weights_np)[0]
        return GameState(chosen_move)
