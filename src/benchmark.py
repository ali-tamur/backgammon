import timeit

from src.main import main

runtime = timeit.timeit("main()", globals=globals(), number=1)
print(runtime)
