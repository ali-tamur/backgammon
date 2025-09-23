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


PLAYER_BAR = 24
OPPONENT_BAR = 25
PLAYER_BAG = 26
OPPONENT_BAG = 27
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
        assert on_board + self.a[PLAYER_BAR] + self.a[PLAYER_BAG] == 15

    def debug_count_opponent_stones(self):
        on_board = sum([abs(self.a[i]) for i in range(COLLECT_END) if self.a[i] < 0])
        assert on_board + self.a[OPPONENT_BAR] + self.a[OPPONENT_BAG] == 15

    @staticmethod
    def game_start():
        a = (2, 0, 0, 0, 0, -5,
             0, -3, 0, 0, 0, 5,
             -5, 0, 0, 0, 3, 0,
             5, 0, 0, 0, 0, -2,
             0, 0, 0, 0)
        return GameState(a)

    def update_board(self, src_index: int, dst_index: int):
        a = list(self.a)
        a[src_index] -= 1
        if a[dst_index] == -1:
            a[OPPONENT_BAR] += 1
            a[dst_index] = 1
        else:
            a[dst_index] += 1
        return GameState(tuple(a))

    def can_move_on_board(self, src_index, dice: DICE) -> bool:
        dst_index = dice + src_index
        return dst_index < COLLECT_END and self.a[dst_index] >= -1

    def moves_for_on_bar(self, dice: Dice):
        r = []
        for d in dice.dice_to_use(False):
            if self.a[d - 1] >= -1:
                new_state = self.update_board(PLAYER_BAR, d - 1)
                new_dice = dice.dice_is_used(d)
                r.append((new_state, new_dice))
        return r

    def playable_indexes_for_collect(self, dice: DICE) -> List[int]:
        src_indexes = [index for index in range(COLLECT_START, COLLECT_END) if self.a[index] > 0]
        can_collect_with_waste = all([index + dice >= COLLECT_END for index in src_indexes])
        if can_collect_with_waste:
            return [min(src_indexes)]
        cands = [src_index for src_index in src_indexes if self.can_move_on_board(src_index, dice)]
        if COLLECT_END - dice in src_indexes:
            cands.append(COLLECT_END - dice)
        return cands

    def playable_indexes_for_normal(self, dice: DICE) -> List[int]:
        return [i for i in range(COLLECT_END) if self.a[i] > 0 and self.can_move_on_board(i, dice)]

    def playable_indexes(self, dice: DICE):
        if self.move_state() == STATE.COLLECTING:
            return self.playable_indexes_for_collect(dice)
        else:
            return self.playable_indexes_for_normal(dice)

    def opponent_state(self):
        a = [-self.a[i] for i in range(COLLECT_END - 1, -1, -1)]
        for i in [OPPONENT_BAR, PLAYER_BAR, OPPONENT_BAG, PLAYER_BAG]:
            a.append(self.a[i])
        return GameState(tuple(a))

    def player_on_bar(self):
        return self.a[PLAYER_BAR]

    def opponent_on_bar(self):
        return self.a[OPPONENT_BAR]

    def player_collected(self):
        return self.a[PLAYER_BAG]

    def opponent_collected(self):
        return self.a[OPPONENT_BAG]

    def _score_type(self):
        if self.a[OPPONENT_BAR] > 0 or any(self.a[i] < 0 for i in range(COLLECT_START, COLLECT_END)):
            return 3
        if self.a[OPPONENT_BAG] == 0:
            return 2
        return 1

    def score(self):
        if self.a[PLAYER_BAG] == 15:
            return self._score_type()
        if self.a[OPPONENT_BAG] == 15:
            return -self.opponent_state()._score_type()
        return None

    def move_state(self) -> STATE:
        if self.player_on_bar() > 0:
            return STATE.ON_BAR
        if sum([self.a[i] for i in range(COLLECT_START, COLLECT_END) if self.a[i] > 0]) + self.a[PLAYER_BAG] == 15:
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
