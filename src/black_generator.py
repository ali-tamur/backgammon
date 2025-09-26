from typing import Tuple, List

from src.state import *


def _on_board(index):
    return 0 <= index < COLLECT_END


def _can_move_on_board(a: List[int], src_index, dice: int) -> bool:
    dst_index = src_index - dice
    return dst_index >= 0 and a[dst_index] <= 1


def _playable_indexes_for_normal(a: List[int], dice: int) -> List[int]:
    return [i for i in range(COLLECT_END) if a[i] < 0 <= i - dice and a[i - dice] <= 1]


def _moves_for_on_bar(a: List[int], dice: List[int]) -> List[
    Tuple[GameState, List[int]]]:
    r = []
    dice_list = [dice[-1]] if dice[0] == dice[-1] else dice
    for d in dice_list:
        land_index = COLLECT_END - d
        if a[land_index] <= 1:
            new_state = update_black(a[:], BLACK_BAR, land_index)
            new_dice = dice[:]
            new_dice.remove(d)
            r.append((new_state, new_dice))
    return r


def _playable_indexes_for_collect(a: List[int], dice: int) -> List[int]:
    src_indexes = [i for i in range(6) if a[i] < 0]
    if max(src_indexes) - dice < 0:
        return [max(src_indexes)]
    cands = [src_index for src_index in src_indexes if _can_move_on_board(a, src_index, dice)]
    perfect = dice - 1
    if perfect in src_indexes:
        cands.append(perfect)
    return cands


def _move_state(a) -> STATE:
    if a[BLACK_BAR] > 0:
        return STATE.ON_BAR
    related_stones = -sum([a[i] for i in range(6) if a[i] < 0])
    related_stones += a[BLACK_BAG]
    return STATE.COLLECTING if related_stones == 15 else STATE.NORMAL


def generate_black_moves(initial_state: GameState, initial_dice: List[int]) -> List[List[int]]:
    partial_results = [(initial_state.a, initial_dice)]
    result_list = [initial_state.a]
    result_dice = len(initial_dice)
    i = 0
    while i < len(partial_results):
        a, dice = partial_results[i]
        i += 1
        if not dice:
            if result_dice > 0:
                result_list, result_dice = [a], 0
            else:
                result_list.append(a)
            continue
        board_state = _move_state(a)
        if board_state == STATE.ON_BAR:
            candidates = _moves_for_on_bar(a, dice)
            if not candidates:
                this_dice = len(dice)
                if this_dice < result_dice:
                    result_list, result_dice = [a], this_dice
                elif this_dice == result_dice:
                    result_list.append(a)
                continue
            for new_state, new_dice in candidates:
                if not new_dice:
                    if result_dice > 0:
                        result_list, result_dice = [a], 0
                    else:
                        result_list.append(a)
                else:
                    partial_results.append((new_state, new_dice))
        else:
            if board_state == STATE.COLLECTING or dice[-1] == dice[0]:
                dices = [dice[-1]]
            else:
                dices = dice
            for d in dices:
                if board_state == STATE.NORMAL:
                    playable = _playable_indexes_for_normal(a, d)
                else:
                    playable = _playable_indexes_for_collect(a, d)
                if not playable:
                    this_dice = len(dice)
                    if this_dice < result_dice:
                        result_list, result_dice = [a], this_dice
                    elif this_dice == result_dice:
                        result_list.append(a)
                for src_index in playable:
                    dst_index = src_index - d
                    if not _on_board(dst_index):
                        new_state = update_black(a[:], src_index, BLACK_BAG)
                        if score_state(new_state) is not None:
                            return [new_state]
                    else:
                        new_state = update_black(a[:], src_index, dst_index)
                    new_dice = dice[:]
                    new_dice.remove(d)
                    if not new_dice:
                        if result_dice > 0:
                            result_list, result_dice = [new_state], 0
                        else:
                            result_list.append(new_state)
                    else:
                        partial_results.append((new_state, new_dice))
    return result_list
