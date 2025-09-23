import random
from abc import ABC, abstractmethod

import keras
from keras import Model
import tensorflow as tf
from src.training_data import state_to_tensor
import numpy as np

from src.dice import Dice
from src.move_generator import generate_moves
from src.state import GameState


class Player(ABC):
    def __init__(self, name):
        self.name = name
        pass

    @abstractmethod
    def move(self, state, dice):
        pass


class RandomPlayer(Player):
    def __init__(self, name, r: random.Random):
        super().__init__(name)
        self.r = r

    def move(self, state: GameState, dice: Dice) -> GameState:
        moves = list(generate_moves(state, dice))
        return self.r.choice(moves)

class NNPlayer(Player):
    def __init__(self, name, model:keras.Model, r: random.Random, temperature):
        super().__init__(name)
        self.r = r
        self.model = model
        self.temperature = temperature

    def move(self, state: GameState, dice: Dice) -> GameState:
        moves = list(generate_moves(state, dice))
        pure_weights = [(self.model.predict(tf.expand_dims(state_to_tensor(st.a), axis=0), verbose=0)[0][0])/self.temperature for st in moves]
        exp_weights = np.exp(pure_weights)

        return self.r.choices(moves, weights=exp_weights)[0]
