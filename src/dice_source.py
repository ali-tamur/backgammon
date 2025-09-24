import random
from typing import List


class Dice:
    def __init__(self, random_seed):
        self.r = random.Random(random_seed)

    @staticmethod
    def to_list(d1, d2) -> List[int]:
        if d1 == d2:
            return [d1, d1, d1, d1]
        elif d1 > d2:
            return [d2, d1]
        else:
            return [d1, d2]

    def throw_dice(self):
        d1 = self.r.randint(1, 6)
        d2 = self.r.randint(1, 6)
        return Dice.to_list(d1, d2)
