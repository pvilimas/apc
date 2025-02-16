#!/usr/bin/python3

import os
from io import StringIO
import sys
import random
from random import randint
import subprocess
from dataclasses import dataclass
import numpy

N = 1000
DIGITS = "0123456789abcdefghjiklmnopqrstuvwxyz"

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

def random_bigstr(base: int) -> str:
    all_digits = "0123456789abcdefghijklmnopqrstuvwxyz"
    valid_digits = all_digits[0:base]
    l = random.choices(valid_digits, k=randint(1,50))
    s = "".join(l)
    return s

def random_digit() -> Expr:
    return Expr(str(randint(1, 10**randint(1,5))))

# a single operation
def random_short_expr():
    return random.choices([
            random_bignum().add(random_bignum()),
            random_bignum().sub(random_bignum()),
            random_bignum().mul(random_bignum()),
            random_bignum().intdiv(random_digit()),
            random_bignum().mod(random_digit()),
        ], weights=[1,1,1,4,3], k=1)[0]

def run_test_apc():
    passed = 0

    for i in range(N):
        e = random_short_expr()

        py_answer = str(eval(e.py_expr))
        apc_answer = test_apc(e.apc_expr)
        apc_answer = apc_answer.strip('\n')

        if py_answer == apc_answer:
            passed += 1
        # else:
            print(f"{e.apc_expr=}\n"
                f"{py_answer=}\n"
                f"{apc_answer=}\n")

    print(f"passed {passed} / {N}")

def base(num,b,numerals="0123456789abcdefghijklmnopqrstuvwxyz"):
    return ((num == 0) and numerals[0]) or (baseN(num // b, b,
        numerals).lstrip(numerals[0]) + numerals[num % b])

def run_test_apc_base_conv():
    passed = 0

    for i in range(N):

        b0 = randint(2,36)
        num_str = random_bigstr(b0)
        num_int = int(num_str, b0)
        b1 = randint(2,36)

        apc_expr = f"{num_str}_{b0} # {b1}"

        py_answer = f"{numpy.base_repr(num_int, base=b1)}"
        apc_answer = test_apc(apc_expr).strip('\n').split("_")[0]

        if py_answer == apc_answer:
            passed += 1
        else:
            print(f"{apc_expr=  }\n"
                f"{py_answer= }\n"
                f"{apc_answer=}\n")
    print(f"passed {passed} / {N}")

def main():
    run_test_apc_base_conv()

if __name__ == '__main__':
    main()
