from typing import Tuple, List

from src.state import *
from line_profiler import LineProfiler, profile


def update_black(a, src_index: int, dst_index: int):
    a[src_index] += 1
    if a[dst_index] == 1:
        a[WHITE_BAR] += 1
        a[dst_index] -= 2
    else:
        a[dst_index] -= 1
    return a


def _moves_for_on_bar(a: List[int], dice: int) -> List[Tuple[List[int], int]]:
    r = []
    d1, d2 = dice % 10, (dice // 10) % 10
    d = d1
    while d != 0:
        land_index = COLLECT_END - d
        if a[land_index] <= 1:
            new_state = update_black(a[:], BLACK_BAR, land_index)
            new_dice = dice // 10 if d == d1 else d1
            r.append((new_state, new_dice))
        d = d2 if d == d1 and d1 != d2 else 0
    return r


def _playable_indexes_for_collect(a: List[int], dice: int) -> List[int]:
    src_indexes = [i for i in range(6) if a[i] < 0]
    if src_indexes[-1] < dice:
        return [src_indexes[-1]]
    cands = [src_index for src_index in src_indexes if src_index-dice>=0 and a[src_index-dice] <= 1]
    if dice-1 in src_indexes:
        cands.append(dice-1)
    return cands


@profile
def generate_black_moves(initial_state: GameState, initial_dice: int ) -> List[List[int]]:
    result_list = []
    result_dice = 2 if initial_dice < 100 else 4

    home_stones = sum(initial_state.a[i] for i in range(6) if initial_state.a[i] < 0)
    partial_results: List[Tuple[List[int], int, int]] = [(initial_state.a, initial_dice, home_stones)]

    while partial_results:
        a, dice, home_stones = partial_results.pop()
        if dice == 0:
            if result_dice > 0:
                result_list, result_dice = [a], 0
            else:
                result_list.append(a)
            continue
        board_state = STATE.ON_BAR if a[BLACK_BAR] < 0 else STATE.COLLECTING if home_stones + a[BLACK_BAG] == -15 else STATE.NORMAL
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
                    partial_results.append((new_state, new_dice, home_stones))
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
                    playable = [i for i in range(d,COLLECT_END) if a[i] < 0 and a[i - d] <= 1]
                else:
                    playable = _playable_indexes_for_collect(a, d)
                if not playable:
                    dice_size = 0 if not dice else 1 if dice < 10 else 2 if dice < 100 else 3 if dice < 1000 else 4
                    if dice_size < result_dice:
                        result_list, result_dice = [a], dice_size
                    elif dice_size == result_dice:
                        result_list.append(a)
                for src_index in playable:
                    dst_index = src_index - d
                    new_state = a[:]
                    new_home_stones = home_stones
                    if dst_index < 0:
                        update_black(new_state, src_index, BLACK_BAG)
                        new_home_stones += 1
                        if score_state(new_state) is not None:
                            return [new_state]
                    else:
                        update_black(new_state, src_index, dst_index)
                        if src_index >= 6 > dst_index:
                            new_home_stones -= 1
                    if not new_dice:
                        if result_dice > 0:
                            result_list, result_dice = [new_state], 0
                        else:
                            result_list.append(new_state)
                    else:
                        partial_results.append((new_state, new_dice, new_home_stones))
                d = d2 if d == d1 else 0
    return result_list
