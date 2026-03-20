"""
M++ Lexer — Tokenizes M++ source code
"""
import re
from dataclasses import dataclass
from enum import Enum
from typing import List, Optional


class TokenType(Enum):
    # Literals
    NUMBER = "NUMBER"
    STRING = "STRING"
    IDENT = "IDENT"
    TRUE = "TRUE"
    FALSE = "FALSE"

    # Keywords
    LET = "LET"
    CONST = "CONST"
    FN = "FN"
    IF = "IF"
    ELIF = "ELIF"
    ELSE = "ELSE"
    FOR = "FOR"
    WHILE = "WHILE"
    IN = "IN"
    RETURN = "RETURN"
    CLASS = "CLASS"
    PUB = "PUB"
    PRIV = "PRIV"
    SELF = "SELF"
    NEW = "NEW"

    # Operators
    PLUS = "PLUS"
    MINUS = "MINUS"
    STAR = "STAR"
    SLASH = "SLASH"
    PERCENT = "PERCENT"
    EQ = "EQ"
    EQEQ = "EQEQ"
    NEQ = "NEQ"
    LT = "LT"
    LE = "LE"
    GT = "GT"
    GE = "GE"
    AND = "AND"
    OR = "OR"
    NOT = "NOT"
    ARROW = "ARROW"

    # Delimiters
    LPAREN = "LPAREN"
    RPAREN = "RPAREN"
    LBRACE = "LBRACE"
    RBRACE = "RBRACE"
    LBRACKET = "LBRACKET"
    RBRACKET = "RBRACKET"
    COMMA = "COMMA"
    DOT = "DOT"
    COLON = "COLON"
    SEMICOLON = "SEMICOLON"

    EOF = "EOF"


@dataclass
class Token:
    type: TokenType
    value: any
    line: int
    column: int


KEYWORDS = {
    "let": TokenType.LET,
    "const": TokenType.CONST,
    "fn": TokenType.FN,
    "if": TokenType.IF,
    "elif": TokenType.ELIF,
    "else": TokenType.ELSE,
    "for": TokenType.FOR,
    "while": TokenType.WHILE,
    "in": TokenType.IN,
    "return": TokenType.RETURN,
    "class": TokenType.CLASS,
    "pub": TokenType.PUB,
    "priv": TokenType.PRIV,
    "self": TokenType.SELF,
    "new": TokenType.NEW,
    "true": TokenType.TRUE,
    "false": TokenType.FALSE,
}


class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1

    def current(self) -> str:
        if self.pos >= len(self.source):
            return "\0"
        return self.source[self.pos]

    def peek(self, offset: int = 1) -> str:
        p = self.pos + offset
        if p >= len(self.source):
            return "\0"
        return self.source[p]

    def advance(self) -> str:
        if self.pos >= len(self.source):
            return "\0"
        ch = self.source[self.pos]
        self.pos += 1
        if ch == "\n":
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return ch

    def skip_whitespace(self):
        while self.current() in " \t\n\r":
            self.advance()

    def skip_comment(self):
        if self.current() == "/" and self.peek() == "/":
            while self.current() != "\n" and self.current() != "\0":
                self.advance()

    def read_number(self) -> Token:
        start_line, start_col = self.line, self.column
        num_str = ""
        while self.current().isdigit() or self.current() == ".":
            num_str += self.advance()
        val = float(num_str) if "." in num_str else int(num_str)
        return Token(TokenType.NUMBER, val, start_line, start_col)

    def read_string(self, quote: str) -> Token:
        start_line, start_col = self.line, self.column
        self.advance()  # consume opening quote
        s = ""
        while self.current() != quote and self.current() != "\0":
            if self.current() == "\\":
                self.advance()
                esc = self.advance()
                s += {"n": "\n", "t": "\t", '"': '"', "'": "'", "\\": "\\"}.get(esc, esc)
            else:
                s += self.advance()
        if self.current() == quote:
            self.advance()
        return Token(TokenType.STRING, s, start_line, start_col)

    def read_ident(self) -> Token:
        start_line, start_col = self.line, self.column
        ident = ""
        while self.current().isalnum() or self.current() == "_":
            ident += self.advance()
        tok_type = KEYWORDS.get(ident, TokenType.IDENT)
        return Token(tok_type, ident, start_line, start_col)

    def tokenize(self) -> List[Token]:
        tokens = []
        while self.pos < len(self.source):
            self.skip_whitespace()
            if self.pos >= len(self.source):
                break
            self.skip_comment()
            if self.pos >= len(self.source):
                break
            ch = self.current()
            start_line, start_col = self.line, self.column

            # Two-char operators
            if ch == "=" and self.peek() == "=":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.EQEQ, "==", start_line, start_col))
                continue
            if ch == "!" and self.peek() == "=":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.NEQ, "!=", start_line, start_col))
                continue
            if ch == "<" and self.peek() == "=":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.LE, "<=", start_line, start_col))
                continue
            if ch == ">" and self.peek() == "=":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.GE, ">=", start_line, start_col))
                continue
            if ch == "-" and self.peek() == ">":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.ARROW, "=>", start_line, start_col))
                continue
            if ch == "=" and self.peek() == ">":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.ARROW, "=>", start_line, start_col))
                continue
            if ch == "&" and self.peek() == "&":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.AND, "&&", start_line, start_col))
                continue
            if ch == "|" and self.peek() == "|":
                self.advance()
                self.advance()
                tokens.append(Token(TokenType.OR, "||", start_line, start_col))
                continue

            # Single char
            if ch == "+":
                tokens.append(Token(TokenType.PLUS, ch, start_line, start_col))
                self.advance()
            elif ch == "-":
                tokens.append(Token(TokenType.MINUS, ch, start_line, start_col))
                self.advance()
            elif ch == "*":
                tokens.append(Token(TokenType.STAR, ch, start_line, start_col))
                self.advance()
            elif ch == "/":
                tokens.append(Token(TokenType.SLASH, ch, start_line, start_col))
                self.advance()
            elif ch == "%":
                tokens.append(Token(TokenType.PERCENT, ch, start_line, start_col))
                self.advance()
            elif ch == "=":
                tokens.append(Token(TokenType.EQ, ch, start_line, start_col))
                self.advance()
            elif ch == "<":
                tokens.append(Token(TokenType.LT, ch, start_line, start_col))
                self.advance()
            elif ch == ">":
                tokens.append(Token(TokenType.GT, ch, start_line, start_col))
                self.advance()
            elif ch == "!":
                tokens.append(Token(TokenType.NOT, ch, start_line, start_col))
                self.advance()
            elif ch == "(":
                tokens.append(Token(TokenType.LPAREN, ch, start_line, start_col))
                self.advance()
            elif ch == ")":
                tokens.append(Token(TokenType.RPAREN, ch, start_line, start_col))
                self.advance()
            elif ch == "{":
                tokens.append(Token(TokenType.LBRACE, ch, start_line, start_col))
                self.advance()
            elif ch == "}":
                tokens.append(Token(TokenType.RBRACE, ch, start_line, start_col))
                self.advance()
            elif ch == "[":
                tokens.append(Token(TokenType.LBRACKET, ch, start_line, start_col))
                self.advance()
            elif ch == "]":
                tokens.append(Token(TokenType.RBRACKET, ch, start_line, start_col))
                self.advance()
            elif ch == ",":
                tokens.append(Token(TokenType.COMMA, ch, start_line, start_col))
                self.advance()
            elif ch == ".":
                tokens.append(Token(TokenType.DOT, ch, start_line, start_col))
                self.advance()
            elif ch == ":":
                tokens.append(Token(TokenType.COLON, ch, start_line, start_col))
                self.advance()
            elif ch == ";":
                tokens.append(Token(TokenType.SEMICOLON, ch, start_line, start_col))
                self.advance()
            elif ch in '"\'':
                tokens.append(self.read_string(ch))
            elif ch.isdigit():
                tokens.append(self.read_number())
            elif ch.isalpha() or ch == "_":
                tokens.append(self.read_ident())
            else:
                self.advance()  # skip unknown

        tokens.append(Token(TokenType.EOF, None, self.line, self.column))
        return tokens
