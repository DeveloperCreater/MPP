#!/usr/bin/env python3
"""
M++ Interpreter — Run M++ programs
Usage: python run.py <file.mpp>
       python run.py -e "print('Hello, M++')"
"""
import sys
import os

# Add mpp package to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from mpp import run, run_file
from mpp.parser import ParseError
from mpp.interpreter import MppRuntimeError


def main():
    if len(sys.argv) < 2:
        print("M++ — The Ultimate Computer Language")
        print("Usage: python run.py <file.mpp>")
        print('       python run.py -e "print(42)"')
        sys.exit(1)

    if sys.argv[1] == "-e":
        if len(sys.argv) < 3:
            print("Error: -e requires code string")
            sys.exit(1)
        code = sys.argv[2]
        try:
            run(code)
        except ParseError as e:
            print(f"Parse Error: {e}")
            sys.exit(1)
        except MppRuntimeError as e:
            print(f"Runtime Error: {e}")
            sys.exit(1)
    else:
        path = sys.argv[1]
        if not os.path.exists(path):
            print(f"Error: File not found: {path}")
            sys.exit(1)
        try:
            run_file(path)
        except ParseError as e:
            print(f"Parse Error: {e}")
            sys.exit(1)
        except MppRuntimeError as e:
            print(f"Runtime Error: {e}")
            sys.exit(1)


if __name__ == "__main__":
    main()
