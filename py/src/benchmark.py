import timeit

from py.src.main import main

runtime = timeit.timeit("main()", globals=globals(), number=1)
print(runtime)
