from typing import List, Set

from dice import Dice, DICE
from state import *

"""
Section: Make a move and return a new game state
"""


def _to_board(state: GameState, src_index: int, dst_index: int) -> GameState:
    """
    :param state: a GameState
    :param src_index: index where the stone moves from. -1 if moving from bar.
    :param src_index: index where to put the stone. Assumes it is on the board (not collected)
    :return: a new game state

    Assumes the move is legal.
    """
    a = list(state.a)
    if src_index >= 0:
        a[src_index] -= 1
    else:
        a[PLAYER_BAR] -= 1
    if a[dst_index] == -1:
        a[OPPONENT_BAR] += 1
        a[dst_index] = 1
    else:
        a[dst_index] += 1
    return GameState(tuple(a))


def from_bar_to_board(state: GameState, index: int) -> GameState:
    return _to_board(state, -1, index)


def from_board_to_board(state: GameState, src_index: int, dst_index: int) -> GameState:
    return _to_board(state, src_index, dst_index)


def from_board_to_bag(state: GameState, src_index: int) -> GameState:
    """
    :param state: a GameState
    :param src_index: where the stone moves from
    :return: a new game state

    Assumes the move is legal.
    """
    a = list(state.a)
    a[src_index] -= 1
    a[PLAYER_BAG] += 1
    return GameState(tuple(a))


"""
Section: Check if a move destination is valid
"""


def can_move_on_board(state: GameState, src_index, dice: DICE) -> bool:
    """
    :param state: a game state; a stone is trying to move normally
    :param src_index: the index of the moving stone
    :param dice: the dice
    :return: whether the destination is valid
    """
    dst_index = dice + src_index
    return state.a[src_index] > 0 and dst_index < COLLECT_END and state.a[dst_index] >= -1


def can_land_from_bar(state, dice) -> bool:
    """
    :param state: an on-bar game state; a stone is trying to land
    :param dice: the dice
    :return: whether the destination is valid
    """
    return state.a[dice - 1] >= -1


"""
Section: Generate moves for a single dice
"""


def moves_for_on_bar(state: GameState, dice: Dice) -> List[GameState]:
    """
    :param state: A game state where the STATE == ON_BAR
    :param dice: the dice to use
    :return: list of legal states after playing the dice. It might be empty.
    """
    r = []
    for d in dice.dice_to_use(state):
        if can_land_from_bar(state, d):
            new_state = from_bar_to_board(state, d - 1)
            new_dice = dice.dice_is_used(d)
            r.append((new_state, new_dice))
    return r


def playable_indexes_for_collect(state: GameState, dice: DICE) -> List[int]:
    """
    :param state: A game state where the STATE == COLLECTING
    :param dice:  the dice to use
    :return: list of legal indexes that can be played. It can be empty.
    """
    src_indexes = [index for index in range(COLLECT_START, COLLECT_END) if state.a[index] > 0]
    can_collect_with_waste = all([index + dice >= COLLECT_END for index in src_indexes])
    if can_collect_with_waste:
        return [min(src_indexes)]
    cands = []
    if COLLECT_END - dice in src_indexes:
        cands.append(COLLECT_END - dice)
    cands += [src_index for src_index in src_indexes if can_move_on_board(state, src_index, dice)]
    return cands


def playable_indexes_for_normal(state: GameState, dice: DICE) -> List[int]:
    """
    :param state: A game state where the STATE == NORMAL
    :param dice:  the dice to use
    :return: list of legal states after playing the dice. It can be empty.
    """
    return [i for i in range(COLLECT_END) if state.a[i] > 0 and can_move_on_board(state, i, dice)]


def playable_indexes(state: GameState, dice: DICE):
    if state.move_state() == STATE.COLLECTING:
        return playable_indexes_for_collect(state, dice)
    else:
        return playable_indexes_for_normal(state, dice)


"""
Section: generate moves from a dice roll for any game state.
"""


def generate_moves(initial_state: GameState, initial_dice: Dice) -> Set[GameState]:
    """
    :param initial_state: any unfinished game state
    :param initial_dice: Dice object that is the result of a dice throw
    :return: List of game states after each legal move.
    """
    partial_results = [(initial_state, initial_dice)]
    result = {(initial_state, len(initial_dice))}
    while partial_results:
        state, dice = partial_results.pop()
        if state.score() is not None:
            return {state}
        if dice.is_empty():
            result.add((state, 0))
            continue
        move_state = state.move_state()
        if move_state == STATE.ON_BAR:
            candidates = moves_for_on_bar(state, dice)
            if not candidates:
                result.add((state, len(dice)))
                continue
            partial_results += candidates
        else:
            dices = dice.dice_to_use(state)
            for d in dices:
                playable = playable_indexes(state, d)
                if not playable:
                    result.add((state, len(dice)))
                for src_index in playable:
                    if d + src_index >= COLLECT_END:
                        new_state = from_board_to_bag(state, src_index)
                    else:
                        new_state = from_board_to_board(state, src_index, src_index + d)
                    partial_results.append((new_state, dice.dice_is_used(d)))
    min_remaining = min([remaining for _, remaining in result])
    moves = {state for state, remaining in result if remaining == min_remaining}
    return moves
