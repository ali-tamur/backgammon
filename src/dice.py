from typing import List, Literal, TypeAlias

from src.state import STATE, GameState

DICE: TypeAlias = Literal[1, 2, 3, 4, 5, 6]


class Dice:
    def __init__(self, dice_list: List[DICE]):
        self.dice_list = dice_list

    @staticmethod
    def dice_throw(d1: DICE, d2: DICE):
        if d1 == d2:
            return Dice([d1, d1, d1, d1])
        return Dice([d1, d2]) if d1 < d2 else Dice([d2, d1])

    def dice_to_use(self, state: GameState) -> List[DICE]:
        if len(self.dice_list) == 0:
            return []
        if len(self.dice_list) == 1 or self.dice_list[0] == self.dice_list[-1]:
            return [self.dice_list[0]]
        if state.move_state() == STATE.COLLECTING:
            return [self.dice_list[-1]]
        return self.dice_list

    def dice_is_used(self, dice_value):
        new_list = self.dice_list[:]
        assert dice_value in self.dice_list
        new_list.remove(dice_value)
        return Dice(new_list)

    def is_empty(self):
        return not self.dice_list

    def __len__(self):
        return len(self.dice_list)

    def __repr__(self):
        return self.dice_list.__repr__()
