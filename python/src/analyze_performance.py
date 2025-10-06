import cProfile
import pstats
import random

from python.src.cc_generator import generate_black_moves, generate_white_moves
from python.src.dice_source import Dice
from python.src.logger import DummyLogWriter
from python.src.main import play_games
from python.src.player import RandomPlayer


def my_game_engine():
    random_seed = random.Random(31415)

    alice = RandomPlayer("alice", random_seed, generate_white_moves)
    bob = RandomPlayer("bob", random_seed, generate_black_moves)
    play_games(alice, bob, Dice(random_seed), 200, DummyLogWriter(), False)


cProfile.run("my_game_engine()", "profile.pstats")
p = pstats.Stats("profile.pstats")

p.sort_stats("cumtime").print_stats()
