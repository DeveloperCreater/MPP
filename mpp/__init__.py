"""
M++ — The Ultimate Computer Language
Combines C++, JavaScript, and Python
"""
from .lexer import Lexer, Token, TokenType
from .parser import Parser, ParseError
from .ast import Program
from .interpreter import Interpreter, MppRuntimeError


def run(source: str) -> None:
    """Run M++ source code."""
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()
    interpreter = Interpreter()
    interpreter.run(ast)


def run_file(path: str) -> None:
    """Run an M++ file."""
    with open(path, "r", encoding="utf-8") as f:
        run(f.read())
