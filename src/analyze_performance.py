import cProfile
import pstats
import random

from src.main import play_games
from src.dice_source import DiceSource
from src.player import RandomPlayer
from src.move_generator import WhiteGeneratorFactory, BlackGeneratorFactory
from src.logger import DummyLogWriter

def my_game_engine():
    random_seed = random.Random(31415)

    alice = RandomPlayer('alice', random_seed, WhiteGeneratorFactory)
    bob = RandomPlayer('bob', random_seed, BlackGeneratorFactory)
    play_games(alice, bob, DiceSource(random_seed), 100, DummyLogWriter(), False)

cProfile.run('my_game_engine()', 'profile_output.pstats')
p = pstats.Stats('profile_output.pstats')

p.sort_stats('cumtime').print_stats('src/')
