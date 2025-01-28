#!/usr/bin/python3

import os
from io import StringIO
import sys
import random
from random import randint
import subprocess
from dataclasses import dataclass

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

def random_expr() -> Expr:
    r1 = randint(0, 5)
    if r1 <= 2:
        # X_VALUE, V_NUMBER

        # base
        # r2 = randint(2,36)
        r2 = random.choice([2,7,8,10,16,31])
        allowed_digits = DIGITS[0:r2-1]

        # num digits
        r3 = randint(1, 35)

        s = ""
        for i in range(r3):
            s += random.choice(allowed_digits)

        # sign
        r4 = randint(0, 1)
        if r4:
            s = f"-{s}"

        apc_s = s
        if r3 != 10:
            apc_s = f"{s}_{r2}"
        return Expr(str(int(s,r2)), s)
    elif r1 == 3:
        # X_UNOP

        # operation
        r2 = randint(0, 1)
        re0 = random_expr()

        op = "+-"[r2]
        e = Expr(f"{re0.py_expr}", f"{re0.apc_expr}")

        # explicit parens
        r3 = randint(0, 1)
        if r3:
            e = Expr(f"({e.py_expr})", f"({e.apc_expr})")
        e = Expr(f"{op}{e.py_expr}", f"{op}{e.apc_expr}")
        return e

    elif r1 >= 4:
        # X_BINOP

        # operation
        r2 = randint(0, 2)
        re0 = random_expr()
        re1 = random_expr()
        if r2 == 0:
            return Expr(f"{re0.py_expr} + {re1.py_expr}", f"{re0.apc_expr} + {re1.apc_expr}")
        elif r2 == 1:
            return Expr(f"{re0.py_expr} - {re1.py_expr}", f"{re0.apc_expr} - {re1.apc_expr}")
        elif r2 == 2:

            # multiplication form
            r3 = randint(0, 3)

            # TODO
            r3 = 3

            return Expr(
                f"{re0.py_expr} * {re1.py_expr}",
                    [f"{re0.apc_expr}({re1.apc_expr})",
                    f"({re0.apc_expr})({re1.apc_expr})",
                    f"({re0.apc_expr}){re1.apc_expr}",
                    f"{re0.apc_expr} * {re1.apc_expr}"][r3])

def run_test_apc():
    passed = 0

    for i in range(N):
        e = random_expr()

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

def main():
    run_test_apc()

if __name__ == '__main__':
    main()
