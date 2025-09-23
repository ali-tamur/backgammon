import pytest

from src.dice import Dice
from src.state import *


class TestDice:
    def test_1(self):
        dice = Dice.dice_throw(3, 5)
        assert dice.dice_list == [3, 5]
        assert len(dice) == 2
        assert not dice.is_empty()

        dice = Dice.dice_throw(5, 3)
        assert dice.dice_list == [3, 5]
        assert len(dice) == 2
        assert not dice.is_empty()

        dice = Dice.dice_throw(2, 2)
        assert dice.dice_list == [2, 2, 2, 2]
        assert len(dice) == 4
        assert not dice.is_empty()

    def test_normal(self):
        dice = Dice.dice_throw(3, 5)
        state = GameState.game_start()
        assert dice.dice_to_use(False) == [3, 5]

        dice = Dice.dice_throw(5, 3)
        state = GameState.game_start()
        assert dice.dice_to_use(False) == [3, 5]

        dice = Dice.dice_throw(6, 6)
        state = GameState.game_start()
        assert dice.dice_to_use(False) == [6]

    def test_collecting(self):
        dice = Dice.dice_throw(3, 5)
        assert dice.dice_to_use(True) == [5]
        assert len(dice) == 2

        dice = Dice.dice_throw(6, 4)
        assert dice.dice_to_use(True) == [6]
        assert len(dice) == 2

        dice = Dice.dice_throw(4, 4)
        assert dice.dice_to_use(True) == [4]
        assert len(dice) == 4

    def test_use_dice(self):
        dice = Dice.dice_throw(1, 2)
        state = GameState.game_start()
        assert dice.dice_to_use(False) == [1, 2]
        new_dice = dice.dice_is_used(1)
        assert dice.dice_to_use(False) == [1, 2]
        assert new_dice.dice_to_use(False) == [2]
        assert len(new_dice) == 1
        assert not new_dice.is_empty()
        new_dice_2 = new_dice.dice_is_used(2)
        assert new_dice_2.dice_to_use(False) == []
        assert new_dice_2.is_empty()
        with pytest.raises(AssertionError):
            dice.dice_is_used(4)
        with pytest.raises(AssertionError):
            new_dice.dice_is_used(1)
        with pytest.raises(AssertionError):
            new_dice_2.dice_is_used(5)
