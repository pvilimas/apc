#!/usr/bin/python3

import os
from io import StringIO
import sys
import random
from random import randint
import subprocess
from dataclasses import dataclass
import numpy

N = 100
DIGITS = "0123456789abcdefghjiklmnopqrstuvwxyz"
FAKE, MP, REAL = 0, 1, 2
BASES = {
    2:  [2, 31, 2147483648],
    3:  [3, 20, 3486784401],
    4:  [4, 15, 1073741824],
    5:  [5, 13, 1220703125],
    6:  [6, 12, 2176782336],
    7:  [7, 11, 1977326743],
    8:  [8, 10, 1073741824],
    9:  [9, 10, 3486784401],
    10: [10, 9, 1000000000],
    11: [11, 9, 2357947691],
    12: [12, 8, 429981696],
    13: [13, 8, 815730721],
    14: [14, 8, 1475789056],
    15: [15, 8, 2562890625],
    16: [16, 7, 268435456],
    17: [17, 7, 410338673],
    18: [18, 7, 612220032],
    19: [19, 7, 893871739],
    20: [20, 7, 1280000000],
    21: [21, 7, 1801088541],
    22: [22, 7, 2494357888],
    23: [23, 7, 3404825447],
    24: [24, 6, 191102976],
    25: [25, 6, 244140625],
    26: [26, 6, 308915776],
    27: [27, 6, 387420489],
    28: [28, 6, 481890304],
    29: [29, 6, 594823321],
    30: [30, 6, 729000000],
    31: [31, 6, 887503681],
    32: [32, 6, 1073741824],
    33: [33, 6, 1291467969],
    34: [34, 6, 1544804416],
    35: [35, 6, 1838265625],
    36: [36, 6, 2176782336],
}

def run_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout

# returns the output of apc as a string
def test_apc(test_input: str) -> str:
    result = subprocess.run(
        ['build/apc', test_input],
        shell=False,
        capture_output=True,
        text=True)

    lines = result.stdout.split('\n')
    return lines[-2].strip('\n =')

@dataclass
class Expr:
    py_expr: str
    apc_expr: str

    def __init__(self, p, a=None):
        self.py_expr = p
        self.apc_expr = a or p

    def add(self, e):
        return Expr(f"{self.py_expr} + {e.py_expr}",
                    f"{self.apc_expr} + {e.apc_expr}")

    def sub(self, e):
        return Expr(f"{self.py_expr} - {e.py_expr}",
                    f"{self.apc_expr} - {e.apc_expr}")

    def mul(self, e):
        return Expr(f"{self.py_expr} * {e.py_expr}",
                    f"{self.apc_expr} * {e.apc_expr}")

    def intdiv(self, e):
        return Expr(f"{self.py_expr} // {e.py_expr}",
                    f"{self.apc_expr} / {e.apc_expr}")

    def mod(self, e):
        return Expr(f"{self.py_expr} % {e.py_expr}",
                    f"{self.apc_expr} % {e.apc_expr}")

def random_bignum() -> Expr:
    return Expr(str(randint(1, 10**randint(1,50))))

def random_bigstr(base: int, n_digits: int = 50) -> str:
    all_digits = "0123456789abcdefghijklmnopqrstuvwxyz"
    valid_digits = all_digits[0:base]
    l = random.choices(valid_digits, k=n_digits)
    s = "".join(l)
    return s

def random_digit() -> Expr:
    return Expr(str(randint(1, 10**randint(1,8))))

# a single operation
def random_short_expr():
    return random.choices([
            random_bignum().add(random_bignum()),
            random_bignum().sub(random_bignum()),
            random_bignum().mul(random_bignum()),
            random_bignum().intdiv(random_digit()),
            random_bignum().mod(random_digit()),
        ], weights=[1,1,1,1,1], k=1)[0]

def run_test_apc():
    passed = 0

    for i in range(N):
        e = random_short_expr()

        py_answer = str(eval(e.py_expr))
        apc_answer = test_apc(e.apc_expr)
        apc_answer = apc_answer.strip('\n')

        if py_answer == apc_answer:
            passed += 1
        else:
            print(f"{e.apc_expr=}\n"
                f"{py_answer=}\n"
                f"{apc_answer=}\n")

    print(f"passed {passed} / {N}")

def base(num,b,numerals="0123456789abcdefghijklmnopqrstuvwxyz"):
    return ((num == 0) and numerals[0]) or (baseN(num // b, b,
        numerals).lstrip(numerals[0]) + numerals[num % b])

def run_test_apc_base_conv():
    passed = 0
    total_passed = 0
    total = 0

    for b0 in range(2,37):
        for b1 in range(2,37):
            if b0 == b1:
                continue
            # b0, b1 = 6, 3
            if BASES[b0][REAL] > BASES[b1][REAL]:
                print('[old>new] ', end="")
            elif BASES[b0][REAL] < BASES[b1][REAL]:
                print('[old<new] ', end="")
            else:
                print('[old=new] ', end="")
            for i in range(N):
                num_str = random_bigstr(b0)
                num_int = int(num_str, b0)

                apc_expr = f"{num_str}_{b0} # {b1}"

                py_answer = f"{numpy.base_repr(num_int, base=b1)}"
                apc_answer = test_apc(apc_expr).strip('\n').split("_")[0]

                if py_answer == apc_answer:
                    passed += 1
                else:
                    ...
                    print(f"{apc_expr=  }\n"
                        f"{py_answer= }\n"
                        f"{apc_answer=}\n")
            print(f"{b0} => {b1}: passed {passed} / {N}")
            total_passed += passed
            total += N
            passed = 0

    print(f"total: passed {total_passed} / {total}")

def main():
    run_test_apc_base_conv()

if __name__ == '__main__':
    main()
