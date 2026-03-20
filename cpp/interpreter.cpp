#include "interpreter.hpp"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <array>

namespace mpp {

Interpreter::Interpreter() { initBuiltins(); }

void Interpreter::initBuiltins() {
    globals["print"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        std::cout << "\n";
    });
    globals["len"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.empty()) throw RuntimeError("len() requires 1 argument");
        const auto& a = args[0];
        if (a.type == Value::Type::String) globals["__len_result"] = Value(static_cast<Number>(a.strVal.size()));
        else if (a.type == Value::Type::Array) globals["__len_result"] = Value(static_cast<Number>(a.arrVal->size()));
        else throw RuntimeError("len() expects string or array");
    });
    globals["range"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.empty()) throw RuntimeError("range() requires 1 argument");
        int n = static_cast<int>(args[0].numVal);
        Value arr; arr.type = Value::Type::Array; arr.arrVal = std::make_shared<std::vector<Value>>();
        for (int i = 0; i < n; i++) arr.arrVal->push_back(Value(static_cast<Number>(i)));
        globals["__range_result"] = arr;
    });
    globals["typeof"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.empty()) { globals["__typeof_result"] = Value("undefined"); return; }
        const auto& a = args[0];
        switch (a.type) {
            case Value::Type::Null: globals["__typeof_result"] = Value("null"); break;
            case Value::Type::Number: globals["__typeof_result"] = Value("number"); break;
            case Value::Type::String: globals["__typeof_result"] = Value("string"); break;
            case Value::Type::Bool: globals["__typeof_result"] = Value("bool"); break;
            case Value::Type::Array: globals["__typeof_result"] = Value("array"); break;
            case Value::Type::Object: globals["__typeof_result"] = Value("object"); break;
            case Value::Type::UserFunc:
            case Value::Type::Native: globals["__typeof_result"] = Value("function"); break;
            case Value::Type::RawPtr: globals["__typeof_result"] = Value("ptr"); break;
        }
    });
    // Low-level: raw memory allocation (C++ power)
    globals["mem_alloc"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.empty()) throw RuntimeError("mem_alloc(size) requires size");
        size_t n = static_cast<size_t>(args[0].numVal);
        Value v; v.type = Value::Type::RawPtr;
        v.rawPtr = std::make_shared<RawMem>(n);
        globals["__mem_alloc_result"] = v;
    });
    globals["mem_read8"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.size() < 2) throw RuntimeError("mem_read8(ptr, offset)");
        if (args[0].type != Value::Type::RawPtr) throw RuntimeError("Expected ptr");
        size_t off = static_cast<size_t>(args[1].numVal);
        if (off >= args[0].rawPtr->size) throw RuntimeError("Offset out of bounds");
        globals["__mem_read_result"] = Value(static_cast<Number>(args[0].rawPtr->data[off]));
    });
    globals["mem_write8"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.size() < 3) throw RuntimeError("mem_write8(ptr, offset, value)");
        if (args[0].type != Value::Type::RawPtr) throw RuntimeError("Expected ptr");
        size_t off = static_cast<size_t>(args[1].numVal);
        if (off >= args[0].rawPtr->size) throw RuntimeError("Offset out of bounds");
        args[0].rawPtr->data[off] = static_cast<uint8_t>(args[2].numVal);
    });

    globals["input"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (!args.empty()) std::cout << args[0].toString();
        std::string line;
        std::getline(std::cin, line);
        globals["__input_result"] = Value(line);
    });

    // Interop: run external commands (call other languages/tools)
    // exec("node my.js") -> stdout string
    globals["exec"] = Value([this](Interpreter&, const std::vector<Value>& args) {
        if (args.empty()) throw RuntimeError("exec(cmd) requires cmd string");
        std::string cmd = args[0].toString();
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe) throw RuntimeError("exec() failed to start process");
        std::string out;
        std::array<char, 4096> buffer{};
        while (true) {
            size_t n = fread(buffer.data(), 1, buffer.size(), pipe);
            if (n == 0) break;
            out.append(buffer.data(), n);
        }
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        globals["__exec_result"] = Value(out);
    });
}

Value Interpreter::resolve(const std::string& name) {
    auto it = scope.find(name);
    if (it != scope.end()) return it->second;
    it = globals.find(name);
    if (it != globals.end()) return it->second;
    throw RuntimeError("Undefined variable: " + name);
}

void Interpreter::assign(Expr* target, const Value& val) {
    if (auto* id = dynamic_cast<IdentExpr*>(target)) {
        if (scope.count(id->name)) scope[id->name] = val;
        else globals[id->name] = val;
        return;
    }
    if (auto* idx = dynamic_cast<IndexExpr*>(target)) {
        Value obj = evalExpr(idx->obj.get());
        Value i = evalExpr(idx->index.get());
        if (obj.type == Value::Type::Array)
            (*obj.arrVal)[static_cast<size_t>(i.numVal)] = val;
        else throw RuntimeError("Cannot assign to index");
        return;
    }
    if (auto* m = dynamic_cast<MemberExpr*>(target)) {
        Value obj = evalExpr(m->obj.get());
        if (obj.type == Value::Type::Object) {
            (*obj.objVal)[m->member] = val;
        } else throw RuntimeError("Cannot assign to member");
        return;
    }
    throw RuntimeError("Invalid assignment target");
}

Value Interpreter::evalExpr(Expr* expr) {
    if (auto* e = dynamic_cast<NumberExpr*>(expr)) return Value(e->value);
    if (auto* e = dynamic_cast<StringExpr*>(expr)) return Value(e->value);
    if (auto* e = dynamic_cast<BoolExpr*>(expr)) return Value(e->value);
    if (auto* e = dynamic_cast<IdentExpr*>(expr)) return resolve(e->name);
    if (auto* e = dynamic_cast<BinaryExpr*>(expr)) return evalBinary(e);
    if (auto* e = dynamic_cast<UnaryExpr*>(expr)) return evalUnary(e);
    if (auto* e = dynamic_cast<CallExpr*>(expr)) return evalCall(e);
    if (auto* e = dynamic_cast<IndexExpr*>(expr)) {
        Value obj = evalExpr(e->obj.get());
        Value idx = evalExpr(e->index.get());
        if (obj.type == Value::Type::Array)
            return (*obj.arrVal)[static_cast<size_t>(idx.numVal)];
        if (obj.type == Value::Type::String)
            return Value(String(1, obj.strVal[static_cast<size_t>(idx.numVal)]));
        throw RuntimeError("Cannot index");
    }
    if (auto* e = dynamic_cast<MemberExpr*>(expr)) {
        Value obj = evalExpr(e->obj.get());
        if (obj.type == Value::Type::Object) {
            auto it = obj.objVal->find(e->member);
            return it != obj.objVal->end() ? it->second : Value();
        }
        throw RuntimeError("Member access on non-object");
    }
    if (auto* e = dynamic_cast<ArrayExpr*>(expr)) {
        Value arr; arr.type = Value::Type::Array; arr.arrVal = std::make_shared<std::vector<Value>>();
        for (auto& el : e->elements) arr.arrVal->push_back(evalExpr(el.get()));
        return arr;
    }
    if (auto* e = dynamic_cast<ObjectExpr*>(expr)) {
        Value obj; obj.type = Value::Type::Object; obj.objVal = std::make_shared<std::map<std::string, Value>>();
        for (auto& p : e->pairs) (*obj.objVal)[p.first] = evalExpr(p.second.get());
        return obj;
    }
    if (auto* e = dynamic_cast<LambdaExpr*>(expr))
        return makeClosure(e, scope);
    return Value();
}

Value Interpreter::evalBinary(BinaryExpr* e) {
    Value left = evalExpr(e->left.get());
    Value right = evalExpr(e->right.get());
    if (e->op == "+") {
        if (left.type == Value::Type::String || right.type == Value::Type::String)
            return Value(left.toString() + right.toString());
        return Value(left.numVal + right.numVal);
    }
    if (e->op == "-") return Value(left.numVal - right.numVal);
    if (e->op == "*") return Value(left.numVal * right.numVal);
    if (e->op == "/") return Value(left.numVal / right.numVal);
    if (e->op == "%") return Value(std::fmod(left.numVal, right.numVal));
    if (e->op == "==") return Value(left.equals(right));
    if (e->op == "!=") return Value(!left.equals(right));
    if (e->op == "<") return Value(left.numVal < right.numVal);
    if (e->op == "<=") return Value(left.numVal <= right.numVal);
    if (e->op == ">") return Value(left.numVal > right.numVal);
    if (e->op == ">=") return Value(left.numVal >= right.numVal);
    if (e->op == "&&") return Value(left.isTruthy() && right.isTruthy());
    if (e->op == "||") return Value(left.isTruthy() || right.isTruthy());
    return Value();
}

Value Interpreter::evalUnary(UnaryExpr* e) {
    Value right = evalExpr(e->right.get());
    if (e->op == "-") return Value(-right.numVal);
    if (e->op == "!") return Value(!right.isTruthy());
    return Value();
}

Value Interpreter::evalCall(CallExpr* e) {
    Value callee;
    Value memberObj;
    bool isMethodCall = false;
    if (auto* m = dynamic_cast<MemberExpr*>(e->callee.get())) {
        memberObj = evalExpr(m->obj.get());
        if (memberObj.type == Value::Type::Object) {
            auto it = memberObj.objVal->find(m->member);
            if (it != memberObj.objVal->end()) { callee = it->second; isMethodCall = true; }
        }
        if (!isMethodCall) callee = evalExpr(e->callee.get());
    } else callee = evalExpr(e->callee.get());

    std::vector<Value> args;
    if (isMethodCall) args.push_back(memberObj);
    for (auto& a : e->args) args.push_back(evalExpr(a.get()));

    if (callee.type == Value::Type::Native) {
        callee.nativeVal(*this, args);
        if (globals.count("__range_result")) { Value r = globals["__range_result"]; globals.erase("__range_result"); return r; }
        if (globals.count("__len_result")) { Value r = globals["__len_result"]; globals.erase("__len_result"); return r; }
        if (globals.count("__typeof_result")) { Value r = globals["__typeof_result"]; globals.erase("__typeof_result"); return r; }
        if (globals.count("__input_result")) { Value r = globals["__input_result"]; globals.erase("__input_result"); return r; }
        if (globals.count("__exec_result")) { Value r = globals["__exec_result"]; globals.erase("__exec_result"); return r; }
        if (globals.count("__mem_alloc_result")) { Value r = globals["__mem_alloc_result"]; globals.erase("__mem_alloc_result"); return r; }
        if (globals.count("__mem_read_result")) { Value r = globals["__mem_read_result"]; globals.erase("__mem_read_result"); return r; }
        if (globals.count("__new_result")) { Value r = globals["__new_result"]; globals.erase("__new_result"); return r; }
        return Value();
    }
    if (callee.type == Value::Type::UserFunc) {
        auto& uf = *callee.userFunc;
        auto oldScope = scope;
        scope = uf.closure;
        for (size_t i = 0; i < uf.params.size(); i++)
            scope[uf.params[i]] = i < args.size() ? args[i] : Value();
        Value result;
        try {
            if (auto* blk = dynamic_cast<BlockStmt*>(uf.body))
                result = execBlock(blk, true);
            else if (auto* es = dynamic_cast<ExprStmt*>(uf.body))
                result = evalExpr(es->expr.get());
        } catch (const ReturnException& ret) { result = ret.value; }
        scope = oldScope;
        return result;
    }
    throw RuntimeError("Not callable");
}

Value Interpreter::makeClosure(LambdaExpr* lambda, std::map<std::string, Value> captures) {
    auto uf = std::make_shared<UserFunc>();
    uf->params = lambda->params;
    uf->closure = captures;
    uf->body = lambda->body.get();
    Value v; v.type = Value::Type::UserFunc; v.userFunc = uf;
    return v;
}

void Interpreter::run(ProgramNode* program) {
    scope.clear();
    for (auto& stmt : program->statements) execStmt(stmt.get());
}

Value Interpreter::execBlock(BlockStmt* block, bool implicitReturn) {
    Value result;
    for (auto& stmt : block->statements) {
        result = execStmt(stmt.get());
    }
    return implicitReturn ? result : Value();
}

Value Interpreter::execStmt(Stmt* stmt) {
    if (auto* v = dynamic_cast<VarDeclStmt*>(stmt)) {
        Value val = evalExpr(v->value.get());
        globals[v->name] = val;
        return Value();
    }
    if (auto* a = dynamic_cast<AssignStmt*>(stmt)) {
        assign(a->target.get(), evalExpr(a->value.get()));
        return Value();
    }
    if (auto* e = dynamic_cast<ExprStmt*>(stmt)) {
        return evalExpr(e->expr.get());
    }
    if (auto* b = dynamic_cast<BlockStmt*>(stmt)) {
        return execBlock(b, false);
    }
    if (auto* i = dynamic_cast<IfStmt*>(stmt)) {
        if (evalExpr(i->condition.get()).isTruthy())
            return execBlock(i->thenBlock.get(), true);
        for (auto& eb : i->elifBlocks) {
            if (evalExpr(eb.first.get()).isTruthy())
                return execBlock(eb.second.get(), true);
        }
        if (i->elseBlock)
            return execBlock(i->elseBlock.get(), true);
        return Value();
    }
    if (auto* f = dynamic_cast<ForStmt*>(stmt)) {
        if (!f->iterVar.empty()) {
            Value iterable = evalExpr(f->iterExpr.get());
            for (auto& item : *iterable.arrVal) {
                scope[f->iterVar] = item;
                execBlock(f->body.get(), false);
            }
        } else {
            if (f->init) execStmt(f->init.get());
            while (!f->condition || evalExpr(f->condition.get()).isTruthy()) {
                execBlock(f->body.get(), false);
                if (f->update) execStmt(f->update.get());
                if (!f->condition) break;
            }
        }
        return Value();
    }
    if (auto* w = dynamic_cast<WhileStmt*>(stmt)) {
        while (evalExpr(w->condition.get()).isTruthy())
            execBlock(w->body.get(), false);
        return Value();
    }
    if (auto* r = dynamic_cast<ReturnStmt*>(stmt)) {
        Value v = r->value ? evalExpr(r->value.get()) : Value();
        throw ReturnException{v};
    }
    if (auto* fd = dynamic_cast<FuncDeclStmt*>(stmt)) {
        auto uf = std::make_shared<UserFunc>();
        uf->params = fd->params;
        uf->closure = scope;
        uf->body = fd->body.get();
        Value v; v.type = Value::Type::UserFunc; v.userFunc = uf;
        globals[fd->name] = v;
        return Value();
    }
    if (auto* c = dynamic_cast<ClassDeclStmt*>(stmt)) {
        defineClass(c);
        return Value();
    }
    return Value();
}

void Interpreter::defineClass(ClassDeclStmt* c) {
    std::string name = c->name;
    ClassDeclStmt* classPtr = c;
    globals[name] = Value([this, classPtr](Interpreter&, const std::vector<Value>& args) {
        Value obj; obj.type = Value::Type::Object; obj.objVal = std::make_shared<std::map<std::string, Value>>();
        for (auto& m : classPtr->members)
            (*obj.objVal)[std::get<1>(m)] = std::get<2>(m) ? evalExpr(std::get<2>(m).get()) : Value();
        for (auto& method : classPtr->methods) {
            auto uf = std::make_shared<UserFunc>();
            uf->params = method->params;
            uf->closure = scope;
            uf->closure["self"] = obj;
            uf->body = method->body.get();
            Value fn; fn.type = Value::Type::UserFunc; fn.userFunc = uf;
            (*obj.objVal)[method->name] = fn;
        }
        auto it = obj.objVal->find("init");
        if (it != obj.objVal->end() && it->second.type == Value::Type::UserFunc) {
            auto& uf = *it->second.userFunc;
            uf.closure["self"] = obj;
            for (size_t i = 0; i < uf.params.size(); i++)
                uf.closure[uf.params[i]] = (i == 0) ? obj : (i - 1 < args.size() ? args[i - 1] : Value());
            auto oldScope = scope;
            scope = uf.closure;
            try {
                if (auto* blk = dynamic_cast<BlockStmt*>(uf.body))
                    execBlock(blk, false);
                else if (auto* es = dynamic_cast<ExprStmt*>(uf.body))
                    evalExpr(es->expr.get());
            } catch (...) { scope = oldScope; throw; }
            scope = oldScope;
        }
        globals["__new_result"] = obj;
    });
}

} // namespace mpp
