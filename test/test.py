#!/usr/bin/python3

import os
from io import StringIO
import sys
from random import randint
import subprocess

N = 100

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

    output = result.stdout[3:] # remove leading " = "
    return output

def random_expr() -> str:
    r1 = randint(0, 5)
    if r1 <= 2:
        # X_VALUE, V_NUMBER

        # number of digits
        r2 = randint(1, 25)

        # 1000000000...
        s = '1' + ''.join(['0' * (r2 - 1)])
        nd = int(s)

        # signbit
        r3 = randint(0, 1)

        if r3 == 0:
            x = randint(0, nd)
            return str(x)
        else:
            x = randint(-nd, 0)
            return str(x)
    elif r1 == 3:
        # X_UNOP

        # operation
        r2 = randint(0, 1)
        if r2 == 0:
            return f"+({random_expr()})"
        elif r2 == 1:
            return f"-({random_expr()})"
    elif r1 >= 4:
        # X_BINOP

        # operation
        r2 = randint(0, 2)
        if r2 == 0:
            return f"({random_expr()}) + ({random_expr()})"
        elif r2 == 1:
            return f"({random_expr()}) - ({random_expr()})"
        elif r2 == 2:
            return f"({random_expr()}) * ({random_expr()})"


def run_test():
    passed = 0

    for i in range(N):
        e = random_expr()

        py_answer = eval(e)
        apc_answer = test_apc(e)
        if apc_answer[-1] == '\n':
            apc_answer = apc_answer[:-1]


        if str(py_answer) == apc_answer:
            passed += 1
        else:
            print(f"\"{e}\":\n"
                f"py: \"{py_answer}\"\n"
                f"apc: \"{apc_answer}\"\n")

    print(f"passed {passed} / {N}")

def main():
    run_test()

if __name__ == '__main__':
    main()
