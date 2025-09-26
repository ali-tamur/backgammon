from typing import Tuple, List

from src.state import *


def _playable_indexes_for_normal(a: List[int], dice: int) -> List[int]:
    return [i for i in range(COLLECT_END)
            if a[i] > 0 and i + dice < COLLECT_END and a[i + dice] >= -1]


def _moves_for_on_bar(a: List[int], dice: int) -> List[Tuple[GameState, int]]:
    r = []
    d1, d2 = dice%10, (dice//10) % 10
    d = d1
    while d != 0:
        land_index = d - 1
        if a[land_index] >= -1:
            new_state = update_white(a[:], WHITE_BAR, land_index)
            new_dice = dice // 10 if d == d1 else d1
            r.append((new_state, new_dice))
        d = d2 if d == d1 and d1 != d2 else 0
    return r


def _playable_indexes_for_collect(a: List[int], dice: int) -> List[int]:
    src_indexes = [i for i in range(COLLECT_START, COLLECT_END) if a[i] > 0]
    if min(src_indexes) + dice >= COLLECT_END:
        return [min(src_indexes)]
    cands = [src_index for src_index in src_indexes if src_index+dice<COLLECT_END and a[src_index+dice] >= -1]
    perfect = COLLECT_END - dice
    if perfect in src_indexes:
        cands.append(perfect)
    return cands


def _move_state(a) -> STATE:
    if a[WHITE_BAR] > 0:
        return STATE.ON_BAR
    related_stones = sum([a[i] for i in range(COLLECT_START, COLLECT_END) if a[i] > 0])
    related_stones += a[WHITE_BAG]
    return STATE.COLLECTING if related_stones == 15 else STATE.NORMAL


def generate_white_moves(initial_state: GameState, initial_dice: int) -> List[List[int]]:
    partial_results = [(initial_state.a, initial_dice)]
    result_list = [initial_state.a]
    result_dice = 2 if initial_dice < 100 else 4
    i = 0
    while i < len(partial_results):
        a, dice = partial_results[i]
        i += 1
        if dice == 0:
            if result_dice > 0:
                result_list, result_dice = [a], 0
            else:
                result_list.append(a)
            continue
        board_state = _move_state(a)
        if board_state == STATE.ON_BAR:
            candidates = _moves_for_on_bar(a, dice)
            if not candidates:
                dice_size = 0 if not dice else 1 if dice < 10 else 2 if dice < 100 else 3 if dice < 1000 else 4
                if dice_size < result_dice:
                    result_list, result_dice = [a], dice_size
                elif dice_size == result_dice:
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
            d1, d2 = dice % 10, (dice // 10) % 10
            d = d1
            while d != 0:
                if d != d1:
                    new_dice = d1
                elif d1 == d2:
                    d2 = 0
                    new_dice = dice // 10
                elif board_state == STATE.COLLECTING:
                    new_dice = d2
                    d2 = 0
                else:
                    new_dice = d2
                if board_state == STATE.NORMAL:
                    playable = _playable_indexes_for_normal(a, d)
                else:
                    playable = _playable_indexes_for_collect(a, d)
                if not playable:
                    dice_size = 0 if not dice else 1 if dice < 10 else 2 if dice < 100 else 3 if dice < 1000 else 4
                    if dice_size < result_dice:
                        result_list, result_dice = [a], dice_size
                    elif dice_size == result_dice:
                        result_list.append(a)
                for src_index in playable:
                    dst_index = src_index + d
                    if dst_index >= COLLECT_END:
                        new_state = update_white(a[:], src_index, WHITE_BAG)
                        if score_state(new_state) is not None:
                            return [new_state]
                    else:
                        new_state = update_white(a[:], src_index, dst_index)
                    if not new_dice:
                        if result_dice > 0:
                            result_list, result_dice = [new_state], 0
                        else:
                            result_list.append(new_state)
                    else:
                        partial_results.append((new_state, new_dice))
                d = d2 if d == d1 else 0
    return result_list
