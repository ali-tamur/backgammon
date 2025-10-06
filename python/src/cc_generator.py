try:
    from cc.src import backgammon as bg  # type: ignore
except ImportError:
    from python.src import backgammon as bg  # type: ignore


def generate_white_moves(board, d1, d2):
    return bg.white_moves(board, d1, d2)


def generate_black_moves(board, d1, d2):
    return bg.black_moves(board, d1, d2)
