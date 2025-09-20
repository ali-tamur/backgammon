from state import GameState
from dice import Dice
from move_generator import generate_moves
from abc import ABC, abstractmethod
import random

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

    def move(self, state: GameState, dice:Dice) -> GameState:
        moves = list(generate_moves(state, dice))
        return self.r.choice(moves)




