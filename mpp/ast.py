"""
M++ AST — Abstract Syntax Tree nodes
"""
from abc import ABC
from dataclasses import dataclass, field
from typing import List, Optional, Any


# Base
class ASTNode(ABC):
    pass


# Expressions
@dataclass
class Number(ASTNode):
    value: float


@dataclass
class String(ASTNode):
    value: str


@dataclass
class Bool(ASTNode):
    value: bool


@dataclass
class Identifier(ASTNode):
    name: str


@dataclass
class BinaryOp(ASTNode):
    left: ASTNode
    op: str
    right: ASTNode


@dataclass
class UnaryOp(ASTNode):
    op: str
    right: ASTNode


@dataclass
class Call(ASTNode):
    callee: ASTNode
    args: List[ASTNode]


@dataclass
class Index(ASTNode):
    obj: ASTNode
    index: ASTNode


@dataclass
class Member(ASTNode):
    obj: ASTNode
    member: str


@dataclass
class Array(ASTNode):
    elements: List[ASTNode]


@dataclass
class Object(ASTNode):
    pairs: List[tuple]  # [(key, value), ...]


@dataclass
class Conditional(ASTNode):
    condition: ASTNode
    then_expr: ASTNode
    else_expr: Optional[ASTNode]


@dataclass
class Lambda(ASTNode):
    params: List[str]
    body: ASTNode


# Statements
@dataclass
class VarDecl(ASTNode):
    name: str
    value: ASTNode
    mutable: bool = True
    type_hint: Optional[str] = None


@dataclass
class Assignment(ASTNode):
    target: ASTNode  # Identifier, Index, or Member
    value: ASTNode


@dataclass
class ExprStmt(ASTNode):
    expr: ASTNode


@dataclass
class Block(ASTNode):
    statements: List[ASTNode]


@dataclass
class IfStmt(ASTNode):
    condition: ASTNode
    then_block: ASTNode
    elif_branches: List[tuple]  # [(cond, block), ...]
    else_block: Optional[ASTNode]


@dataclass
class ForStmt(ASTNode):
    init: Optional[ASTNode]  # VarDecl or Assignment
    condition: Optional[ASTNode]
    update: Optional[ASTNode]
    body: ASTNode
    iter_var: Optional[str] = None
    iter_expr: Optional[ASTNode] = None


@dataclass
class WhileStmt(ASTNode):
    condition: ASTNode
    body: ASTNode


@dataclass
class ReturnStmt(ASTNode):
    value: Optional[ASTNode]


@dataclass
class FuncDecl(ASTNode):
    name: str
    params: List[tuple]  # [(name, type_hint), ...]
    body: ASTNode
    return_type: Optional[str] = None


@dataclass
class ClassDecl(ASTNode):
    name: str
    members: List[tuple]  # [("pub"/"priv", name, value?), ...]
    methods: List[FuncDecl]


@dataclass
class Program(ASTNode):
    statements: List[ASTNode]
