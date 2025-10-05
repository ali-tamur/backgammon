import statistics
import time

from py.src.main import main  # noqa: F401


def benchmark(n=5):
    times = []
    for i in range(n):
        print(f"{i}")
        start = time.perf_counter()
        main()
        elapsed = time.perf_counter() - start
        times.append(elapsed)
        print(f"Run {i + 1}: {elapsed:.3f}s")

    print(f"\nMean: {statistics.mean(times):.3f}s")
    print(f"Stdev: {statistics.stdev(times):.3f}s")
    return times


if __name__ == "__main__":
    benchmark()
