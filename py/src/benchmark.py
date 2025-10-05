import timeit

from py.src.main import main  # noqa: F401

runtime = timeit.timeit("main()", globals=globals(), number=1)
print(runtime)
