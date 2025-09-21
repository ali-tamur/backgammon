import timeit
import json
import os
import fastavro
import numpy
import tensorflow
import keras

import src.dice
import src.dice_source
import src.game_manager
import src.logger
import src.move_generator
import src.player
import src.training_data
from src.main import main

runtime = timeit.timeit("main()", globals=globals(), number=1)
print(runtime)
