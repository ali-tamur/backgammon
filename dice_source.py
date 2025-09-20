from dice import Dice
import random

class DiceSource:
    def __init__(self, random_seed):
        self.r = random.Random(random_seed)

    def throw_dice(self):
        return self.r.randint(1, 6), self.r.randint(1, 6)
