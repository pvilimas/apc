#!/usr/bin/python3

from dataclasses import dataclass

UINT32_MAX = (2**32) - 1
UINT64_MAX = (2**64) - 1

BASES = range(2, 37)

@dataclass
class Base:
    fake: int
    width: int
    real: int
    max_value: int

def print_c_lookup_table():
    s = "Base[] base_lookup_table = {\n"
    for base in BASES:
        n = 1
        b_to_the_n = base

        while (b_to_the_n * base <= UINT32_MAX):
            b_to_the_n *= base
            n += 1

        s += f"\t[{base}] = (Base){{{n}, {b_to_the_n}}},\n"
    s += "};"
    print(s)

# fake to real
def make_py_lookup_table() -> dict[int, Base]:
    d = {}
    for base in BASES:
        n = 1
        b_to_the_n = base

        while (b_to_the_n * base <= UINT32_MAX):
            b_to_the_n *= base
            n += 1

        max_v = b_to_the_n - 1
        d[base] = Base(fake=base, width=n, real=b_to_the_n, max_value=max_v)
    return d

# this operation is useful for multi-digit division and base conversion
def fits_in_u64(x: int, y: int, z: int) -> bool:
    return x + (y * z) <= UINT64_MAX

def test_all_fit_u64():
    d = make_py_lookup_table()

    for b0 in BASES:
        for b1 in BASES:
            for b2 in BASES:
                max0 = d[b0].max_value
                max1 = d[b1].max_value
                max2 = d[b1].max_value
                result = max0 + (max1 * max2)
                fits = result <= UINT64_MAX
                print(f"[{max0} + ({max1} * {max2})] => {fits}:")
                print(f"\t{max0}")
                print(f"\t{max1}")
                print(f"\t{max2}")
                print(f"\t{result}")
                print(f"\t{UINT64_MAX}")
                ...
                # if not fits: print("!")
    print("done")

def main():
    test_all_fit_u64()

if __name__ == '__main__':
    main()
