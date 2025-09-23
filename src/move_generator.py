from typing import Set
from src.state import *

class ResultKeeper:
    def __init__(self, initial_state:GameState, remaining_dice:int)-> None:
        self.d = {initial_state}
        self.remaining_dice = remaining_dice

    def add(self, state, remaining_dice) -> None:
        if remaining_dice > self.remaining_dice:
            return
        elif remaining_dice < self.remaining_dice:
            self.remaining_dice = remaining_dice
            self.d = {state}
        else:
            self.d.add(state)

    def moves(self) -> Set[GameState]:
        return self.d

def generate_moves(initial_state: GameState, initial_dice: Dice) -> Set[GameState]:
    """
    :param initial_state: any unfinished game state
    :param initial_dice: Dice object that is the result of a dice throw
    :return: List of game states after each legal move.
    """
    partial_results = [(initial_state, initial_dice)]
    result = ResultKeeper(initial_state, len(initial_dice))
    i = 0
    while i < len(partial_results):
        state, dice = partial_results[i]
        i += 1
        if dice.is_empty():
            result.add(state, 0)
            continue
        move_state = state.move_state()
        if move_state == STATE.ON_BAR:
            candidates = state.moves_for_on_bar(dice)
            if not candidates:
                result.add(state, len(dice))
                continue
            for new_state, new_dice in candidates:
                if new_dice.is_empty():
                    result.add(new_state, 0)
                else:
                    partial_results.append((new_state, new_dice))
        else:
            dices = dice.dice_to_use(move_state == STATE.COLLECTING)
            for d in dices:
                if move_state == STATE.NORMAL:
                    playable = state.playable_indexes_for_normal(d)
                else:
                    playable = state.playable_indexes_for_collect(d)
                if not playable:
                    result.add(state, len(dice))
                for src_index in playable:
                    if d + src_index >= COLLECT_END:
                        new_state = state.update_board(src_index, PLAYER_BAG)
                        if new_state.score() is not None:
                            return {new_state}
                    else:
                        new_state = state.update_board(src_index, src_index + d)
                    new_dice = dice.dice_is_used(d)
                    if dice.is_empty():
                        result.add(new_state, 0)
                    else:
                        partial_results.append((new_state, new_dice))
    return result.moves()
