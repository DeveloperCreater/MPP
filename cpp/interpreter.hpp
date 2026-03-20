#pragma once

#include "value.hpp"
#include "ast.hpp"
#include <string>
#include <map>
#include <stdexcept>

namespace mpp {

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

struct ReturnException {
    Value value;
};

struct MppObject {
    std::string className;
    std::map<std::string, Value> fields;
};

struct Interpreter {
    std::map<std::string, Value> globals;
    std::map<std::string, Value> scope;

    Interpreter();
    void run(ProgramNode* program);
    Value evalExpr(Expr* expr);
    Value execStmt(Stmt* stmt);
    Value execBlock(BlockStmt* block, bool implicitReturn = false);

private:
    void initBuiltins();
    Value resolve(const std::string& name);
    void assign(Expr* target, const Value& val);
    Value evalBinary(BinaryExpr* e);
    Value evalUnary(UnaryExpr* e);
    Value evalCall(CallExpr* e);
    Value makeClosure(LambdaExpr* lambda, std::map<std::string, Value> captures);
    void defineClass(ClassDeclStmt* c);
};

} // namespace mpp
