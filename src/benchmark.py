import timeit

runtime = timeit.timeit("main()", globals=globals(), number=1)
print(runtime)
