from enum import Enum, auto
from typing import List

from src.dice import Dice, DICE

"""
Game state is represented by an array of length 28.

The state will always represent the game from the view point of the player who
collects the stones in the lower corner.

The board is a[0..23] 0 is the farthest point, 23 is the one that collects.
positive n : the player's stone
negative n: opponent's stone
a[24]: # of stones of the player on the bar
a[25]: # of stones of the opponent on the bar
a[26]: # of stones the player collected.
a[27]: # of stones the opponent collected.
"""


class STATE(Enum):
    ON_BAR = auto()
    NORMAL = auto()
    COLLECTING = auto()


WHITE_BAR = 24
BLACK_BAR = 25
WHITE_BAG = 26
BLACK_BAG = 27
COLLECT_START = 18
COLLECT_END = 24


class GameState:
    def __init__(self, a):
        self.a = a
        if __debug__:
            self.debug_count_player_stones()
            self.debug_count_opponent_stones()

    def debug_count_player_stones(self):
        on_board = sum([self.a[i] for i in range(COLLECT_END) if self.a[i] > 0])
        assert on_board + self.a[WHITE_BAR] + self.a[WHITE_BAG] == 15

    def debug_count_opponent_stones(self):
        on_board = sum([abs(self.a[i]) for i in range(COLLECT_END) if self.a[i] < 0])
        assert on_board + self.a[BLACK_BAR] + self.a[BLACK_BAG] == 15

    @staticmethod
    def game_start():
        a = (2, 0, 0, 0, 0, -5,
             0, -3, 0, 0, 0, 5,
             -5, 0, 0, 0, 3, 0,
             5, 0, 0, 0, 0, -2,
             0, 0, 0, 0)
        return GameState(a)

    def opponent_state(self):
        a = [-self.a[i] for i in range(COLLECT_END - 1, -1, -1)]
        for i in [BLACK_BAR, WHITE_BAR, BLACK_BAG, WHITE_BAG]:
            a.append(self.a[i])
        return GameState(tuple(a))

    def player_on_bar(self):
        return self.a[WHITE_BAR]

    def opponent_on_bar(self):
        return self.a[BLACK_BAR]

    def player_collected(self):
        return self.a[WHITE_BAG]

    def opponent_collected(self):
        return self.a[BLACK_BAG]

    def _score_type(self):
        if self.a[BLACK_BAR] > 0 or any(self.a[i] < 0 for i in range(COLLECT_START, COLLECT_END)):
            return 3
        if self.a[BLACK_BAG] == 0:
            return 2
        return 1

    def score(self):
        if self.a[WHITE_BAG] == 15:
            return self._score_type()
        if self.a[BLACK_BAG] == 15:
            return -self.opponent_state()._score_type()
        return None

    def move_state(self) -> STATE:
        if self.player_on_bar() > 0:
            return STATE.ON_BAR
        if sum([self.a[i] for i in range(COLLECT_START, COLLECT_END) if self.a[i] > 0]) + self.a[WHITE_BAG] == 15:
            return STATE.COLLECTING
        return STATE.NORMAL

    def __copy__(self):
        return self

    def __eq__(self, other):
        if type(self) != type(other):
            return False
        return self.a == other.a

    def __hash__(self):
        return hash(self.a)

    def hash(self):
        return self.__hash__()

    def __repr__(self):
        return self.a.__repr__()
