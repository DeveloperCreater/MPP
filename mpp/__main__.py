"""
M++ CLI — Entry point for: mpp <file.mpp>
"""
import sys
import os

from . import run, run_file
from .parser import ParseError
from .interpreter import MppRuntimeError


def main():
    args = sys.argv[1:]
    if not args:
        print("M++ — The Ultimate Computer Language")
        print("Usage: mpp <file.mpp>")
        print("       mpp -r <file.mpp>")
        print('       mpp -e "print(42)"')
        sys.exit(1)

    path = None
    code = None

    if args[0] == "-e":
        if len(args) < 2:
            print("Error: -e requires code string")
            sys.exit(1)
        code = args[1]
    elif args[0] == "-r":
        if len(args) < 2:
            print("Error: -r requires file path")
            sys.exit(1)
        path = args[1]
    else:
        path = args[0]

    try:
        if code is not None:
            run(code)
        else:
            if not os.path.exists(path):
                print(f"Error: File not found: {path}")
                sys.exit(1)
            run_file(path)
    except ParseError as e:
        print(f"Parse Error: {e}")
        sys.exit(1)
    except MppRuntimeError as e:
        print(f"Runtime Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
