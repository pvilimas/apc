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
    # print(f'test_apc("{test_input}") => {output}')
    return output

def run_test_add():
    total = 0
    passed = 0

    for i in range(N):
        for j in range(N):
            x = i + randint(-10000000000000, 100000000000000)
            y = j + randint(-10000000000000, 100000000000000)
            py_answer = x + y
            apc_answer = eval(test_apc(f"{x} + {y}"))

            total += 1
            if py_answer == apc_answer:
                passed += 1
                # print(f"     {x}\n"
                #       f"+    {y}\n"
                #       f"==   {apc_answer}\n")
            else:
                print(f"     {x}\n"
                      f"+    {y}\n"
                      f"!=   {apc_answer}\n"
                      f"==   {py_answer}\n")

    print(f"test_add: passed {passed} / {total}")

def main():
    run_test_add()

if __name__ == '__main__':
    main()
