from typing import List

from py.src.state import GameState, COLOR, WHITE_BAR, BLACK_BAR, WHITE_BAG, BLACK_BAG


def get_char(st: GameState, col: int, row: int) -> str:
    n = st.a[col]
    if n == 0 and row == 0:
        return '.'
    if abs(n) > 5 and row == 4:
        return f"{abs(n)}"
    if n > 0 and n > row:
        return '○'
    if n < 0 and abs(n) > row:
        return '●'
    return ' '


def get_bar(st: GameState, color: COLOR):
    n = st.a[WHITE_BAR] if color == COLOR.WHITE else st.a[BLACK_BAR]
    ch = '○' if color == COLOR.WHITE else '●'
    result = [ch] * (min(n, 3))
    if n > 3:
        result.append(f": {n}")
    return ' '.join(result)


def get_bag(st: GameState, row: int, color: COLOR):
    if color == COLOR.WHITE:
        n = st.a[WHITE_BAG]
        ch = '○'
    else:
        n = st.a[BLACK_BAG]
        ch = '●'
    n -= 5 * row
    if n <= 0:
        return ""
    n = min(n, 5)
    result = [ch] * n
    return ' '.join(result)


def get_row(st: GameState, cols: List[int], row: int):
    s = ["│"] + [get_char(st, c, row) for c in cols] + ["│"]
    return ' '.join(s)


class RowBuilder:
    def __init__(self, st=""):
        self.r = [st]

    def add(self, st):
        for ch in st:
            self.r.append(ch)

    def __repr__(self):
        return ''.join(self.r)


class Screen:
    def __init__(self):
        self.rows = []

    def add(self, row: RowBuilder):
        self.rows.append(row)

    def append(self, index, st):
        self.rows[index].add(st)

    def pp(self):
        return '\n'.join([r.__repr__() for r in self.rows])


def pretty_print(st: GameState) -> str:
    screen = Screen()
    screen.add(RowBuilder("┌─────────────┬┬─────────────┐"))

    top_left_cols = list(range(11, 5, -1))
    top_right_cols = list(range(5, -1, -1))
    for r_i in range(5):
        row = RowBuilder(get_row(st, top_left_cols, r_i))
        row.add(get_row(st, top_right_cols, r_i))
        screen.add(row)
    mid_row = "│             ││             │"
    screen.add(RowBuilder(mid_row))
    bottom_left_cols = list(range(12, 18))
    bottom_right_cols = list(range(18, 24))
    for r_i in range(4, -1, -1):
        row = RowBuilder(get_row(st, bottom_left_cols, r_i))
        row.add(get_row(st, bottom_right_cols, r_i))
        screen.add(row)
    screen.add(RowBuilder("└─────────────┴┴─────────────┘"))

    for r_i in range(3):
        screen.append(r_i + 1, get_bag(st, r_i, COLOR.BLACK))
        screen.append(11 - r_i, get_bag(st, r_i, COLOR.WHITE))

    screen.append(5, get_bar(st, COLOR.BLACK))
    screen.append(6, get_bar(st, COLOR.WHITE))
    return screen.pp()
