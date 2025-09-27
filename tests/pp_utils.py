import unittest
from typing import List, Tuple, Any
from src.state import WHITE_BAR, WHITE_BAG, BLACK_BAG, BLACK_BAR
import sys

def color_int(n: int, color_code: str) -> str:
    """Wraps text in ANSI escape codes for terminal color."""
    reset = "\033[0m"
    return f"\033[{color_code}m{n:>3}{reset}"


def color_white(n: int) -> str:
    return color_int(n, "1;33")


def color_black(n: int) -> str:
    return color_int(n, "1;34")

def pp_cell(n:int, *, dot_for_zero=False, will_print_color=False) -> str:
    if n == 0 and dot_for_zero:
        return " . " if will_print_color else "  ."
    if n == 0:
        return "   " if will_print_color else "     "
    if n > 0:
        return f"{color_white(n)}" if will_print_color else f"{n:>3}"
    elif will_print_color:
        return f"{color_black(-n)}" if will_print_color else f"*{-n:>3}"
    else:
        starred = "*"+str(-n)
        return f"{starred:>3}"


def pp_right_margin(bar, bag) -> str:
    if bar == 0 and bag == 0:
        return ""
    return f"{pp_cell(bar)} : {pp_cell(bag)}"


def pp_row(board:List[int], indexes:List[int], *, will_print_color=False) -> str:
    return ' '.join([pp_cell(board[i], dot_for_zero=True, will_print_color=will_print_color) for i in indexes])

def print_lst(lst:List[Any])->str:
    r = []
    for t in lst:
        r.append(t if isinstance(t, str) else ''.join([t[0] * t[1]]))
    return ''.join(r)

def top_of_board()-> str:
    return print_lst(["┌",("─",24), "┬┬", ("─",24),"┐"])

def middle_of_board() -> str:
    return print_lst(["│", (" ", 24), "││", (" ", 24), "│"])

def bottom_of_board() -> str:
    return print_lst(["└", ("─", 24), "┴┴", ("─", 24), "┘"])

def pretty_print_board(board: List[int], *, will_print_in_color=False) -> str:
    top_left_cols = list(range(11, 5, -1))
    top_right_cols = list(range(5, -1, -1))
    top_str = (f"│{pp_row(board, top_left_cols, will_print_color=will_print_in_color)} ││"
               f"{pp_row(board, top_right_cols, will_print_color=will_print_in_color)} │")
    bottom_left_cols = list(range(12, 18))
    bottom_right_cols = list(range(18, 24))
    bot_str = (f"│{pp_row(board, bottom_left_cols, will_print_color=will_print_in_color)} ││"
               f"{pp_row(board, bottom_right_cols, will_print_color=will_print_in_color)} │")
    result:List[str] =[]
    result.append(top_of_board())
    result.append(top_str)
    result.append(middle_of_board())
    result.append(middle_of_board())
    result.append(bot_str)
    result.append(bottom_of_board())
    result[2] = result[2] + f" {pp_right_margin(board[BLACK_BAR], board[BLACK_BAG])}"
    result[3] = result[3] + f" {pp_right_margin(board[WHITE_BAR], board[WHITE_BAG])}"
    return '\n'.join(result)

if __name__ == '__main__':
    b = [1,0,0,1,1,0,0,1,12,-1,-1,-7,0,0,0,1,-1,1,2,8,3,1,0,0,0,0,0,0]
    sys.stdout.write(pretty_print_board(b, will_print_in_color=True) + '\n')
    sys.stdout.flush()
    print(pretty_print_board(b))
