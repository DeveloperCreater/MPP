"""
M++ Interpreter — Executes AST
"""
import subprocess
from typing import Any, Dict, List, Optional, Callable

from .ast import (
    Program, Number, String, Bool, Identifier,
    BinaryOp, UnaryOp, Call, Index, Member, Array, Object,
    Lambda, Block, VarDecl, Assignment, ExprStmt,
    IfStmt, ForStmt, WhileStmt, ReturnStmt,
    FuncDecl, ClassDecl,
)


class ReturnSignal(Exception):
    def __init__(self, value: Any):
        self.value = value


class MppRuntimeError(Exception):
    def __init__(self, msg: str):
        super().__init__(msg)


class MppObject:
    def __init__(self, class_name: str = ""):
        self.class_name = class_name
        self.fields: Dict[str, Any] = {}

    def __repr__(self):
        return f"<{self.class_name or 'Object'}>"

class MppRawMem:
    def __init__(self, n: int):
        self.buf = bytearray(max(0, int(n)))

    def __repr__(self):
        return f"<ptr size={len(self.buf)}>"


class Interpreter:
    def __init__(self):
        self.globals: Dict[str, Any] = {}
        self.scope: Dict[str, Any] = {}
        self._init_builtins()

    def _init_builtins(self):
        def mpp_print(*args):
            print(*[self._to_str(a) for a in args])

        def mpp_input(prompt=None):
            if prompt is None:
                return input()
            return input(self._to_str(prompt))

        def mpp_exec(cmd: str):
            if not isinstance(cmd, str):
                cmd = self._to_str(cmd)
            try:
                out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT)
                return out.decode("utf-8", errors="replace")
            except subprocess.CalledProcessError as e:
                raise MppRuntimeError(f"exec() failed: {e.output.decode('utf-8', errors='replace')}")

        def mpp_mem_alloc(n):
            return MppRawMem(int(n))

        def mpp_mem_read8(ptr, offset):
            if not isinstance(ptr, MppRawMem):
                raise MppRuntimeError("mem_read8(ptr, offset): ptr must be a pointer")
            off = int(offset)
            if off < 0 or off >= len(ptr.buf):
                raise MppRuntimeError("mem_read8: offset out of bounds")
            return int(ptr.buf[off])

        def mpp_mem_write8(ptr, offset, value):
            if not isinstance(ptr, MppRawMem):
                raise MppRuntimeError("mem_write8(ptr, offset, value): ptr must be a pointer")
            off = int(offset)
            if off < 0 or off >= len(ptr.buf):
                raise MppRuntimeError("mem_write8: offset out of bounds")
            ptr.buf[off] = int(value) & 0xFF
            return None

        def mpp_len(x):
            if isinstance(x, (list, str)):
                return len(x)
            raise MppRuntimeError("len() expects array or string")

        def mpp_range(n):
            return list(range(int(n)))

        def mpp_typeof(x):
            if isinstance(x, bool):
                return "bool"
            if isinstance(x, (int, float)):
                return "number"
            if isinstance(x, str):
                return "string"
            if isinstance(x, list):
                return "array"
            if isinstance(x, MppObject):
                return "object"
            if callable(x):
                return "function"
            return "unknown"

        self.globals["print"] = mpp_print
        self.globals["input"] = mpp_input
        self.globals["exec"] = mpp_exec
        self.globals["mem_alloc"] = mpp_mem_alloc
        self.globals["mem_read8"] = mpp_mem_read8
        self.globals["mem_write8"] = mpp_mem_write8
        self.globals["len"] = mpp_len
        self.globals["range"] = mpp_range
        self.globals["typeof"] = mpp_typeof

    def _to_str(self, x: Any) -> str:
        if x is None:
            return "null"
        if isinstance(x, bool):
            return "true" if x else "false"
        if isinstance(x, float) and x == int(x):
            return str(int(x))
        if isinstance(x, MppObject):
            return str(x)
        return str(x)

    def _eval_expr(self, node) -> Any:
        if isinstance(node, Number):
            return node.value
        if isinstance(node, String):
            return node.value
        if isinstance(node, Bool):
            return node.value
        if isinstance(node, Identifier):
            return self._resolve(node.name)
        if isinstance(node, BinaryOp):
            return self._eval_binary(node)
        if isinstance(node, UnaryOp):
            return self._eval_unary(node)
        if isinstance(node, Call):
            return self._eval_call(node)
        if isinstance(node, Index):
            obj = self._eval_expr(node.obj)
            idx = self._eval_expr(node.index)
            if isinstance(obj, (list, str)):
                return obj[int(idx)]
            if isinstance(obj, MppObject):
                return obj.fields.get(str(idx))
            raise MppRuntimeError("Cannot index type")
        if isinstance(node, Member):
            obj = self._eval_expr(node.obj)
            if isinstance(obj, MppObject):
                return obj.fields.get(node.member)
            raise MppRuntimeError("Member access on non-object")
        if isinstance(node, Array):
            return [self._eval_expr(e) for e in node.elements]
        if isinstance(node, Object):
            o = MppObject()
            for k, v in node.pairs:
                o.fields[k] = self._eval_expr(v)
            return o
        if isinstance(node, Lambda):
            return self._make_closure(node, {})

        raise MppRuntimeError(f"Unknown expression: {type(node)}")

    def _resolve(self, name: str) -> Any:
        if name in self.scope:
            return self.scope[name]
        if name in self.globals:
            return self.globals[name]
        raise MppRuntimeError(f"Undefined variable: {name}")

    def _assign(self, target, value: Any):
        if isinstance(target, Identifier):
            if target.name in self.scope:
                self.scope[target.name] = value
            else:
                self.globals[target.name] = value
            return
        if isinstance(target, Index):
            obj = self._eval_expr(target.obj)
            idx = self._eval_expr(target.index)
            if isinstance(obj, list):
                obj[int(idx)] = value
                return
            if isinstance(obj, MppObject):
                obj.fields[str(idx)] = value
                return
        if isinstance(target, Member):
            obj = self._eval_expr(target.obj)
            if isinstance(obj, MppObject):
                obj.fields[target.member] = value
                return
        raise MppRuntimeError("Invalid assignment target")

    def _eval_binary(self, node: BinaryOp) -> Any:
        left = self._eval_expr(node.left)
        right = self._eval_expr(node.right)
        op = node.op

        if op == "+":
            if isinstance(left, str) or isinstance(right, str):
                return self._to_str(left) + self._to_str(right)
            return left + right
        if op == "-":
            return left - right
        if op == "*":
            return left * right
        if op == "/":
            return left / right
        if op == "%":
            return left % right
        if op == "==":
            return left == right
        if op == "!=":
            return left != right
        if op == "<":
            return left < right
        if op == "<=":
            return left <= right
        if op == ">":
            return left > right
        if op == ">=":
            return left >= right
        if op == "&&":
            return bool(left) and bool(right)
        if op == "||":
            return bool(left) or bool(right)

        raise MppRuntimeError(f"Unknown operator: {op}")

    def _eval_unary(self, node: UnaryOp) -> Any:
        right = self._eval_expr(node.right)
        if node.op == "-":
            return -right
        if node.op == "!":
            return not right
        raise MppRuntimeError(f"Unknown unary operator: {node.op}")

    def _make_closure(self, node: Lambda, captures: Dict[str, Any]) -> Callable:
        params = [p if isinstance(p, str) else p[0] for p in node.params]
        body = node.body

        def closure(*args):
            old_scope = self.scope.copy()
            self.scope = {**captures}
            for i, p in enumerate(params):
                self.scope[p] = args[i] if i < len(args) else None
            try:
                if isinstance(body, ExprStmt):
                    return self._eval_expr(body.expr)
                if isinstance(body, Block):
                    return self._exec_block(body, implicit_return=True)
                return self._eval_expr(body)
            except ReturnSignal as r:
                return r.value
            finally:
                self.scope = old_scope

        return closure

    def _eval_call(self, node: Call) -> Any:
        args = [self._eval_expr(a) for a in node.args]
        # For obj.method(), inject self as first arg
        if isinstance(node.callee, Member):
            obj = self._eval_expr(node.callee.obj)
            callee = self._eval_expr(node.callee)
            if not callable(callee):
                raise MppRuntimeError("Not callable")
            return callee(obj, *args)
        callee = self._eval_expr(node.callee)
        if not callable(callee):
            raise MppRuntimeError("Not callable")
        return callee(*args)

    def _exec_block(self, block: Block, implicit_return: bool = False) -> Optional[Any]:
        result = None
        for stmt in block.statements:
            result = self._exec_stmt(stmt)
        return result if implicit_return else None

    def _exec_stmt(self, stmt) -> Optional[Any]:
        if isinstance(stmt, VarDecl):
            val = self._eval_expr(stmt.value)
            self.scope[stmt.name] = val
        elif isinstance(stmt, Assignment):
            val = self._eval_expr(stmt.value)
            self._assign(stmt.target, val)
        elif isinstance(stmt, ExprStmt):
            val = self._eval_expr(stmt.expr)
            return val
        elif isinstance(stmt, Block):
            return self._exec_block(stmt)
        elif isinstance(stmt, IfStmt):
            cond = self._eval_expr(stmt.condition)
            if cond:
                return self._exec_block(stmt.then_block, implicit_return=True)
            for elif_cond, elif_block in stmt.elif_branches:
                if self._eval_expr(elif_cond):
                    return self._exec_block(elif_block, implicit_return=True)
            if stmt.else_block:
                return self._exec_block(stmt.else_block, implicit_return=True)
        elif isinstance(stmt, ForStmt):
            if stmt.iter_var:
                iterable = self._eval_expr(stmt.iter_expr)
                for item in iterable:
                    self.scope[stmt.iter_var] = item
                    result = self._exec_block(stmt.body)
                    if result is not None:
                        return result
            else:
                if stmt.init:
                    self._exec_stmt(stmt.init)
                while True:
                    if stmt.condition and not self._eval_expr(stmt.condition):
                        break
                    result = self._exec_block(stmt.body)
                    if result is not None:
                        return result
                    if stmt.update:
                        self._exec_stmt(stmt.update)
        elif isinstance(stmt, WhileStmt):
            while self._eval_expr(stmt.condition):
                result = self._exec_block(stmt.body)
                if result is not None:
                    return result
        elif isinstance(stmt, ReturnStmt):
            val = self._eval_expr(stmt.value) if stmt.value else None
            raise ReturnSignal(val)
        elif isinstance(stmt, FuncDecl):
            func = self._make_closure(
                Lambda([(p[0], p[1]) for p in stmt.params], stmt.body),
                self.scope.copy()
            )
            self.globals[stmt.name] = func
        elif isinstance(stmt, ClassDecl):
            self._define_class(stmt)
        else:
            raise MppRuntimeError(f"Unknown statement: {type(stmt)}")
        return None

    def _define_class(self, node: ClassDecl):
        class_name = node.name

        def constructor(*args):
            obj = MppObject(class_name)
            for vis, mname, default in node.members:
                obj.fields[mname] = self._eval_expr(default) if default else None
            init = None
            for m in node.methods:
                if m.name == "init":
                    init = m
                obj.fields[m.name] = self._make_method(m, obj)
            if init:
                init_fn = obj.fields["init"]
                init_fn(obj, *args)  # pass self as first arg
            return obj

        self.globals[class_name] = constructor

    def _make_method(self, method: FuncDecl, obj: MppObject) -> Callable:
        params = [p[0] for p in method.params]
        body = method.body

        def method_fn(*args):
            old_scope = self.scope.copy()
            self.scope = {"self": obj}
            for i, p in enumerate(params):
                self.scope[p] = args[i] if i < len(args) else None
            try:
                if isinstance(body, ExprStmt):
                    return self._eval_expr(body.expr)
                result = self._exec_block(body, implicit_return=True)
                return result
            except ReturnSignal as r:
                return r.value
            finally:
                self.scope = old_scope

        return method_fn

    def run(self, program: Program) -> Optional[Any]:
        self.scope = {}
        result = None
        for stmt in program.statements:
            try:
                result = self._exec_stmt(stmt)
            except ReturnSignal as r:
                result = r.value
                break
        return result
