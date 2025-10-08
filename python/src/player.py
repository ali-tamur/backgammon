import random
from abc import ABC, abstractmethod
from collections.abc import Callable

import keras
import numpy as np
import tensorflow as tf

from python.src.state import GameState
from python.src.training_data import state_to_numpy


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
        if len(moves) == 1:
            return GameState(moves[0])
        tensors = tf.stack([state_to_numpy(st) for st in moves])
        raw_scores = self.model(tensors, training=False).numpy().squeeze()
        weights = np.exp(raw_scores / self.temperature)
        chosen_idx = self.r.choices(range(len(moves)), weights=weights)[0]
        return GameState(moves[chosen_idx])
