"""
M++ Parser — Builds AST from tokens
"""
from typing import List, Optional

from .lexer import Token, TokenType, Lexer
from .ast import (
    ASTNode, Program, Number, String, Bool, Identifier,
    BinaryOp, UnaryOp, Call, Index, Member, Array, Object,
    Conditional, Lambda,
    VarDecl, Assignment, ExprStmt, Block, IfStmt, ForStmt,
    WhileStmt, ReturnStmt, FuncDecl, ClassDecl,
)


class ParseError(Exception):
    def __init__(self, msg: str, token: Token):
        super().__init__(f"Line {token.line}: {msg}")
        self.token = token


class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def current(self) -> Token:
        if self.pos >= len(self.tokens):
            return self.tokens[-1]
        return self.tokens[self.pos]

    def advance(self) -> Token:
        t = self.current()
        if self.pos < len(self.tokens):
            self.pos += 1
        return t

    def expect(self, *types: TokenType) -> Token:
        t = self.current()
        if t.type not in types:
            raise ParseError(f"Expected {types}, got {t.type}", t)
        return self.advance()

    def check(self, *types: TokenType) -> bool:
        return self.current().type in types

    def parse(self) -> Program:
        stmts = []
        while not self.check(TokenType.EOF):
            stmt = self.parse_statement()
            if stmt:
                stmts.append(stmt)
            if self.check(TokenType.SEMICOLON):
                self.advance()
        return Program(stmts)

    def parse_statement(self) -> Optional[ASTNode]:
        if self.check(TokenType.LET, TokenType.CONST):
            return self.parse_var_decl()
        if self.check(TokenType.FN):
            return self.parse_func_decl()
        if self.check(TokenType.CLASS):
            return self.parse_class_decl()
        if self.check(TokenType.IF):
            return self.parse_if()
        if self.check(TokenType.FOR):
            return self.parse_for()
        if self.check(TokenType.WHILE):
            return self.parse_while()
        if self.check(TokenType.RETURN):
            self.advance()
            val = None if self.check(TokenType.SEMICOLON, TokenType.RBRACE, TokenType.EOF) else self.parse_expr()
            return ReturnStmt(val)
        if self.check(TokenType.LBRACE):
            return self.parse_block()

        expr = self.parse_expr()
        if expr and self.check(TokenType.EQ) and self._is_assignable(expr):
            self.advance()
            return Assignment(expr, self.parse_expr())
        return ExprStmt(expr) if expr else None

    def _is_assignable(self, node: ASTNode) -> bool:
        return isinstance(node, (Identifier, Index, Member))

    def parse_var_decl(self) -> VarDecl:
        mutable = self.advance().type == TokenType.LET
        name = self.expect(TokenType.IDENT).value
        type_hint = None
        if self.check(TokenType.COLON):
            self.advance()
            type_hint = self.expect(TokenType.IDENT).value
        self.expect(TokenType.EQ)
        value = self.parse_expr()
        return VarDecl(name, value, mutable, type_hint)

    def parse_func_decl(self) -> FuncDecl:
        self.expect(TokenType.FN)
        name = self.expect(TokenType.IDENT).value
        self.expect(TokenType.LPAREN)
        params = []
        while not self.check(TokenType.RPAREN):
            pname = self.expect(TokenType.IDENT, TokenType.SELF).value
            ptype = None
            if self.check(TokenType.COLON):
                self.advance()
                ptype = self.expect(TokenType.IDENT).value
            params.append((pname, ptype))
            if not self.check(TokenType.RPAREN):
                self.expect(TokenType.COMMA)
        self.expect(TokenType.RPAREN)
        ret_type = None
        if self.check(TokenType.COLON):
            self.advance()
            ret_type = self.expect(TokenType.IDENT).value
        if self.check(TokenType.ARROW):
            self.advance()
            body = ExprStmt(self.parse_expr())
        else:
            self.expect(TokenType.LBRACE)
            body = self.parse_block_raw()
        return FuncDecl(name, params, body, ret_type)

    def parse_class_decl(self) -> ClassDecl:
        self.expect(TokenType.CLASS)
        name = self.expect(TokenType.IDENT).value
        self.expect(TokenType.LBRACE)
        members = []
        methods = []
        while not self.check(TokenType.RBRACE, TokenType.EOF):
            if self.check(TokenType.PUB, TokenType.PRIV):
                vis = self.advance().value
                mname = self.expect(TokenType.IDENT).value
                val = None
                if self.check(TokenType.EQ):
                    self.advance()
                    val = self.parse_expr()
                members.append((vis, mname, val))
            elif self.check(TokenType.FN):
                methods.append(self.parse_func_decl())
            else:
                self.advance()
        self.expect(TokenType.RBRACE)
        return ClassDecl(name, members, methods)

    def parse_if(self) -> IfStmt:
        self.expect(TokenType.IF)
        cond = self.parse_expr()
        if self.check(TokenType.LPAREN):
            self.advance()
            self.expect(TokenType.RPAREN)
        then_block = self.parse_block()
        elif_branches = []
        while self.check(TokenType.ELIF):
            self.advance()
            elif_cond = self.parse_expr()
            elif_branch = self.parse_block()
            elif_branches.append((elif_cond, elif_branch))
        else_block = None
        if self.check(TokenType.ELSE):
            self.advance()
            else_block = self.parse_block()
        return IfStmt(cond, then_block, elif_branches, else_block)

    def parse_for(self) -> ForStmt:
        self.expect(TokenType.FOR)
        self.expect(TokenType.LPAREN)
        if self.check(TokenType.LET):
            self.advance()
            iter_var = self.expect(TokenType.IDENT).value
            if self.check(TokenType.IN):
                self.advance()
                iter_expr = self.parse_expr()
                self.expect(TokenType.RPAREN)
                body = self.parse_block()
                return ForStmt(None, None, None, body, iter_var, iter_expr)
            else:
                self.expect(TokenType.EQ)
                init = VarDecl(iter_var, self.parse_expr(), True, None)
        elif self.check(TokenType.IDENT):
            name_tok = self.advance()
            if self.check(TokenType.IN):
                iter_var = name_tok.value
                self.advance()
                iter_expr = self.parse_expr()
                self.expect(TokenType.RPAREN)
                body = self.parse_block()
                return ForStmt(None, None, None, body, iter_var, iter_expr)
            elif self.check(TokenType.EQ):
                self.advance()
                init = Assignment(Identifier(name_tok.value), self.parse_expr())
            else:
                init = ExprStmt(Identifier(name_tok.value))
        else:
            init = self.parse_statement() if not self.check(TokenType.SEMICOLON) else None
        if self.check(TokenType.SEMICOLON):
            self.advance()
        cond = None if self.check(TokenType.SEMICOLON) else self.parse_expr()
        self.expect(TokenType.SEMICOLON)
        update = None if self.check(TokenType.RPAREN) else self.parse_statement()
        self.expect(TokenType.RPAREN)
        body = self.parse_block()
        return ForStmt(init, cond, update, body)

    def parse_while(self) -> WhileStmt:
        self.expect(TokenType.WHILE)
        cond = self.parse_expr()
        if self.check(TokenType.LPAREN):
            self.advance()
            self.expect(TokenType.RPAREN)
        body = self.parse_block()
        return WhileStmt(cond, body)

    def parse_block(self) -> Block:
        self.expect(TokenType.LBRACE)
        return self.parse_block_raw()

    def parse_block_raw(self) -> Block:
        stmts = []
        while not self.check(TokenType.RBRACE, TokenType.EOF):
            stmt = self.parse_statement()
            if stmt:
                stmts.append(stmt)
            if self.check(TokenType.SEMICOLON):
                self.advance()
        self.expect(TokenType.RBRACE)
        return Block(stmts)

    def parse_expr(self) -> ASTNode:
        return self.parse_or()

    def parse_or(self) -> ASTNode:
        left = self.parse_and()
        while self.check(TokenType.OR):
            op = self.advance().value
            left = BinaryOp(left, op, self.parse_and())
        return left

    def parse_and(self) -> ASTNode:
        left = self.parse_equality()
        while self.check(TokenType.AND):
            op = self.advance().value
            left = BinaryOp(left, op, self.parse_equality())
        return left

    def parse_equality(self) -> ASTNode:
        left = self.parse_comparison()
        while self.check(TokenType.EQEQ, TokenType.NEQ):
            op = self.advance().value
            left = BinaryOp(left, op, self.parse_comparison())
        return left

    def parse_comparison(self) -> ASTNode:
        left = self.parse_term()
        while self.check(TokenType.LT, TokenType.LE, TokenType.GT, TokenType.GE):
            op = self.advance().value
            left = BinaryOp(left, op, self.parse_term())
        return left

    def parse_term(self) -> ASTNode:
        left = self.parse_factor()
        while self.check(TokenType.PLUS, TokenType.MINUS):
            op = self.advance().value
            left = BinaryOp(left, op, self.parse_factor())
        return left

    def parse_factor(self) -> ASTNode:
        left = self.parse_unary()
        while self.check(TokenType.STAR, TokenType.SLASH, TokenType.PERCENT):
            op = self.advance().value
            left = BinaryOp(left, op, self.parse_unary())
        return left

    def parse_unary(self) -> ASTNode:
        if self.check(TokenType.MINUS, TokenType.NOT):
            op = self.advance().value
            return UnaryOp(op, self.parse_unary())
        return self.parse_call()

    def parse_call(self) -> ASTNode:
        base = self.parse_primary()
        while True:
            if self.check(TokenType.LPAREN):
                self.advance()
                args = []
                while not self.check(TokenType.RPAREN):
                    args.append(self.parse_expr())
                    if not self.check(TokenType.RPAREN):
                        self.expect(TokenType.COMMA)
                self.expect(TokenType.RPAREN)
                base = Call(base, args)
            elif self.check(TokenType.LBRACKET):
                self.advance()
                idx = self.parse_expr()
                self.expect(TokenType.RBRACKET)
                base = Index(base, idx)
            elif self.check(TokenType.DOT):
                self.advance()
                member = self.expect(TokenType.IDENT).value
                base = Member(base, member)
            else:
                break
        return base

    def parse_primary(self) -> ASTNode:
        if self.check(TokenType.NEW):
            self.advance()
            class_name = self.expect(TokenType.IDENT).value
            self.expect(TokenType.LPAREN)
            args = []
            while not self.check(TokenType.RPAREN):
                args.append(self.parse_expr())
                if not self.check(TokenType.RPAREN):
                    self.expect(TokenType.COMMA)
            self.expect(TokenType.RPAREN)
            return Call(Identifier(class_name), args)
        if self.check(TokenType.NUMBER):
            return Number(self.advance().value)
        if self.check(TokenType.STRING):
            return String(self.advance().value)
        if self.check(TokenType.TRUE):
            self.advance()
            return Bool(True)
        if self.check(TokenType.FALSE):
            self.advance()
            return Bool(False)
        if self.check(TokenType.IDENT, TokenType.SELF):
            return Identifier(self.advance().value)
        if self.check(TokenType.LPAREN):
            self.advance()
            expr = self.parse_expr()
            self.expect(TokenType.RPAREN)
            return expr
        if self.check(TokenType.LBRACKET):
            self.advance()
            elements = []
            while not self.check(TokenType.RBRACKET):
                elements.append(self.parse_expr())
                if not self.check(TokenType.RBRACKET):
                    self.expect(TokenType.COMMA)
            self.expect(TokenType.RBRACKET)
            return Array(elements)
        if self.check(TokenType.LBRACE):
            self.advance()
            pairs = []
            while not self.check(TokenType.RBRACE):
                key = self.expect(TokenType.IDENT).value
                self.expect(TokenType.COLON)
                val = self.parse_expr()
                pairs.append((key, val))
                if not self.check(TokenType.RBRACE):
                    self.expect(TokenType.COMMA)
            self.expect(TokenType.RBRACE)
            return Object(pairs)
        if self.check(TokenType.FN):
            self.advance()
            self.expect(TokenType.LPAREN)
            params = []
            while not self.check(TokenType.RPAREN):
                params.append(self.expect(TokenType.IDENT, TokenType.SELF).value)
                if not self.check(TokenType.RPAREN):
                    self.expect(TokenType.COMMA)
            self.expect(TokenType.RPAREN)
            if self.check(TokenType.ARROW):
                self.advance()
                body = self.parse_expr()
            else:
                body = self.parse_block()
            return Lambda(params, body)
        raise ParseError("Unexpected token", self.current())
