from enum import Enum, auto

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

GAME_START = [2, 0, 0, 0, 0, -5, 0, -3, 0, 0, 0, 5, -5, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, -2, 0, 0, 0, 0]



def score_state(a):
    if a[WHITE_BAG] < 15 and a[BLACK_BAG] > -15:
        return None
    if a[WHITE_BAG] == 15:
        stones_at_home = [i for i in range(COLLECT_START, COLLECT_END) if a[i] < 0]
        if len(stones_at_home)  - a[BLACK_BAR] > 0:
            return 3
        elif a[BLACK_BAG] == 0:
            return 2
        else:
            return 1
    else:
        stones_at_home = [i for i in range(6) if a[i] > 0]
        if a[WHITE_BAR] + len(stones_at_home) > 0:
            return -3
        elif a[WHITE_BAG] == 0:
            return -2
        else:
            return -1


def update_black(a, src_index: int, dst_index: int):
    a[src_index] += 1
    if a[dst_index] == 1:
        a[WHITE_BAR] += 1
        a[dst_index] += -2
    else:
        a[dst_index] += -1
    return a


def update_white(a, src_index: int, dst_index: int):
    a[src_index] += -1
    if a[dst_index] == -1:
        a[BLACK_BAR] -= 1
        a[dst_index] += 2
    else:
        a[dst_index] += 1
    return a


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
        on_board = sum([(self.a[i]) for i in range(COLLECT_END) if self.a[i] < 0])
        assert on_board + self.a[BLACK_BAR] + self.a[BLACK_BAG] == -15

    @staticmethod
    def game_start():
        return GameState(GAME_START)

    def update(self, src_index: int, dst_index: int, color: COLOR):
        if color == COLOR.WHITE:
            return GameState(update_white(self.a[:], src_index, dst_index))
        else:
            return GameState(update_black(self.a[:], src_index, dst_index))

    def score(self):
        return score_state(self.a)

    def __copy__(self):
        return self

    def __eq__(self, other):
        if not isinstance(other, GameState):
            return False
        return self.a == other.a

    def __hash__(self):
        return hash(tuple(self.a))

    def __repr__(self):
        return self.a.__repr__()
