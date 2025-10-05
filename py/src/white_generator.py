
from line_profiler import profile

from py.src.state import *


def update_white(board, src_index: int, dst_index: int):
    board[src_index] += -1
    if board[dst_index] == -1:
        board[BLACK_BAR] -= 1
        board[dst_index] += 2
    else:
        board[dst_index] += 1
    return board


def _moves_for_on_bar(board: list[int], dice: int) -> list[tuple[list[int], int]]:
    r = []
    d1, d2 = dice % 10, (dice // 10) % 10
    d = d1
    while d != 0:
        land_index = d - 1
        if board[land_index] >= -1:
            new_board = update_white(board[:], WHITE_BAR, land_index)
            new_dice = dice // 10 if d == d1 else d1
            r.append((new_board, new_dice))
        d = d2 if d == d1 and d1 != d2 else 0
    return r


def _playable_indexes_for_collect(board: list[int], dice: int) -> list[int]:
    src_indexes = [i for i in range(COLLECT_START, COLLECT_END) if board[i] > 0]
    if src_indexes[0] + dice >= COLLECT_END:
        return [src_indexes[0]]
    cands = [
        src_index
        for src_index in src_indexes
        if src_index + dice < COLLECT_END and board[src_index + dice] >= -1
    ]
    if COLLECT_END - dice in src_indexes:
        cands.append(COLLECT_END - dice)
    return cands


@profile
def generate_white_moves(initial_state: GameState, initial_dice: int) -> list[list[int]]:
    result_list: list[list[int]] = []
    result_dice = 2 if initial_dice < 100 else 4

    home_stones = sum(
        initial_state.a[i] for i in range(COLLECT_START, COLLECT_END) if initial_state.a[i] > 0
    )
    partial_results: list[tuple[list[int], int, int]] = [
        (initial_state.a, initial_dice, home_stones)
    ]
    while partial_results:
        board, dice, home_stones = partial_results.pop()
        if dice == 0:
            if result_dice > 0:
                result_list, result_dice = [board], 0
            else:
                result_list.append(board)
            continue
        board_state = (
            STATE.ON_BAR
            if board[WHITE_BAR] > 0
            else STATE.COLLECTING
            if (home_stones + board[WHITE_BAG] == 15)
            else STATE.NORMAL
        )
        if board_state == STATE.ON_BAR:
            candidates = _moves_for_on_bar(board, dice)
            if not candidates:
                dice_size = (
                    0
                    if not dice
                    else 1
                    if dice < 10
                    else 2
                    if dice < 100
                    else 3
                    if dice < 1000
                    else 4
                )
                if dice_size < result_dice:
                    result_list, result_dice = [board], dice_size
                elif dice_size == result_dice:
                    result_list.append(board)
                continue
            for new_board, new_dice in candidates:
                if not new_dice:
                    if result_dice > 0:
                        result_list, result_dice = [new_board], 0
                    else:
                        result_list.append(new_board)
                else:
                    partial_results.append((new_board, new_dice, home_stones))
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
                    playable = [
                        i for i in range(COLLECT_END - d) if board[i] > 0 and board[i + d] >= -1
                    ]
                else:
                    playable = _playable_indexes_for_collect(board, d)
                if not playable:
                    dice_size = (
                        0
                        if not dice
                        else 1
                        if dice < 10
                        else 2
                        if dice < 100
                        else 3
                        if dice < 1000
                        else 4
                    )
                    if dice_size < result_dice:
                        result_list, result_dice = [board], dice_size
                    elif dice_size == result_dice:
                        result_list.append(board)
                for src_index in playable:
                    dst_index = src_index + d
                    new_board = board[:]
                    new_home_stones = home_stones
                    if dst_index >= COLLECT_END:
                        update_white(new_board, src_index, WHITE_BAG)
                        new_home_stones -= 1
                        if score_state(new_board) is not None:
                            return [new_board]
                    else:
                        update_white(new_board, src_index, dst_index)
                        if src_index < COLLECT_START <= dst_index:
                            new_home_stones += 1
                    if not new_dice:
                        if result_dice > 0:
                            result_list, result_dice = [new_board], 0
                        else:
                            result_list.append(new_board)
                    else:
                        partial_results.append((new_board, new_dice, new_home_stones))
                d = d2 if d == d1 else 0
    return result_list
