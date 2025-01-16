#!/usr/bin/python3

import os
from io import StringIO
import sys
from random import randint
import subprocess

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

def main():
    while True:
        i = input("?= ")
        if i == 'q' or i == 'quit':
            break

        apc_answer = test_apc(i)[:-1]
        py_answer = str(eval(i))

        if apc_answer == py_answer:
            print(f"  correct: \"{apc_answer}\"")
        else:
            print(f"  apc: \"{apc_answer}\"")
            print(f"  py: \"{py_answer}\"")

if __name__ == '__main__':
    main()
