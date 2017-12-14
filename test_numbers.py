from itertools import permutations

numbers = (2, 9, 7, 5, 3)
target = 399

print(next(("{}: {}".format(result, str(order)) for result, order in sorted([((a + b * (c**2) + (d**3) - e), (a, b, c, d, e)) for a, b, c, d, e in permutations(numbers)]) if result == target)))
