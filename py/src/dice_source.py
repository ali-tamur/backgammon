import random


class Dice:
    def __init__(self, random_seed):
        self.r = random.Random(random_seed)

    @staticmethod
    def to_list(d1, d2) -> int:
        if d1 == d2:
            return 1111 * d1
        if d2 < d1:
            d1, d2 = d2, d1
        return 10 * d1 + d2

    def throw_dice(self) -> tuple[int, int]:
        d1 = self.r.randint(1, 6)
        d2 = self.r.randint(1, 6)
        return d1, d2
