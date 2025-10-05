import cProfile
import pstats
import random

from src.main import play_games
from src.dice_source import Dice
from src.player import RandomPlayer
from src.logger import DummyLogWriter
from src.white_generator import generate_white_moves
from src.black_generator import generate_black_moves

from py.src.cc_generator import generate_white_moves, generate_black_moves
def my_game_engine():
    random_seed = random.Random(31415)

    alice = RandomPlayer('alice', random_seed, generate_white_moves)
    bob = RandomPlayer('bob', random_seed, generate_black_moves)
    play_games(alice, bob, Dice(random_seed), 200, DummyLogWriter(), False)

cProfile.run('my_game_engine()', 'profile.pstats')
p = pstats.Stats('profile.pstats')

#p.sort_stats('cumtime').print_stats('src/')
p.sort_stats('cumtime').print_stats()
