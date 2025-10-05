import cProfile
import pstats
import random

from py.src.main import play_games
from py.src.dice_source import Dice
from py.src.player import RandomPlayer
from py.src.logger import DummyLogWriter
from py.src.white_generator import generate_white_moves
from py.src.black_generator import generate_black_moves

from py.src.cc_generator import generate_white_moves, generate_black_moves
def my_game_engine():
    random_seed = random.Random(31415)

    alice = RandomPlayer('alice', random_seed, generate_white_moves)
    bob = RandomPlayer('bob', random_seed, generate_black_moves)
    play_games(alice, bob, Dice(random_seed), 200, DummyLogWriter(), False)

cProfile.run('my_game_engine()', 'profile.pstats')
p = pstats.Stats('profile.pstats')

p.sort_stats('cumtime').print_stats()
