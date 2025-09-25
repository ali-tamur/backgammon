from enum import Enum, auto
from typing import List

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

class COLOR(Enum):
    WHITE = auto()
    BLACK = auto()

WHITE_BAR = 24
BLACK_BAR = 25
WHITE_BAG = 26
BLACK_BAG = 27
COLLECT_START = 18
COLLECT_END = 24

MOD = 1_000_000_007
K = 17
HASH_COEFFICIENTS = [1]
for _ in range(27):
    HASH_COEFFICIENTS.append((HASH_COEFFICIENTS[-1]*K) % MOD)

def get_hash_code(a:List[int]) -> int:
    """ a must be an array of 28 elements.
    """
    assert len(a) == 28
    h = 0
    for i in range(len(a)):
        h += HASH_COEFFICIENTS[i] * a[i]
        h = h % MOD
    return h

GAME_START = [2, 0, 0, 0, 0, -5, 0, -3, 0, 0, 0, 5, -5, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, -2, 0, 0, 0, 0]
GAME_START_HASH = get_hash_code(GAME_START)


class GameState:
    def __init__(self, a, hash_code = 0):
        self.a = a
        self._hash = hash_code or get_hash_code(self.a)
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
        return GameState(GAME_START, GAME_START_HASH)

    def update(self, src_index:int, dst_index:int,color:COLOR):
        a = self.a[:]
        h = self._hash
        difference = []
        if color == COLOR.WHITE or src_index == BLACK_BAR:
            difference.append((src_index, -1))
        else:
            difference.append((src_index, 1))
        if color == COLOR.BLACK:
            if dst_index == BLACK_BAG:
                difference.append((dst_index, 1))
            elif a[dst_index] == 1:
                difference.append((WHITE_BAR, 1))
                difference.append((dst_index, -2))
            else:
                difference.append((dst_index, -1))
        elif a[dst_index] == -1:
                difference.append((BLACK_BAR, 1))
                difference.append((dst_index, 2))
        else:
                difference.append((dst_index, 1))
        for key, value in difference:
            a[key] += value
            h += HASH_COEFFICIENTS[key] * value
        h = h % MOD
        return GameState(a, h)

    def _score_type(self, opponent_bag, opponent_bar, home_range, presence_fn):
        if self.a[opponent_bar] > 0 or any(presence_fn(self.a[i]) for i in home_range):
            return 3
        if self.a[opponent_bag] == 0:
            return 2
        return 1

    def score(self):
        if self.a[WHITE_BAG] < 15 and self.a[BLACK_BAG] < 15:
            return None
        if self.a[WHITE_BAG] == 15:
            return self._score_type(BLACK_BAG, BLACK_BAR, range(COLLECT_START,COLLECT_END),
                                    lambda n: n < 0)
        else:
            return -self._score_type(WHITE_BAG, WHITE_BAR, range(6), lambda n:n>0)

    def __copy__(self):
        return self

    def __eq__(self, other):
        if type(self) != type(other):
            return False
        return self._hash == other._hash and self.a == other.a

    def __hash__(self):
        return self._hash

    def __repr__(self):
        return self.a.__repr__()
