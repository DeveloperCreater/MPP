#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace mpp {

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expr : ASTNode {};
struct Stmt : ASTNode {};

struct NumberExpr : Expr { double value; };
struct StringExpr : Expr { std::string value; };
struct BoolExpr : Expr { bool value; };
struct IdentExpr : Expr { std::string name; };

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
};

struct UnaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> right;
};

struct CallExpr : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
};

struct IndexExpr : Expr {
    std::unique_ptr<Expr> obj;
    std::unique_ptr<Expr> index;
};

struct MemberExpr : Expr {
    std::unique_ptr<Expr> obj;
    std::string member;
};

struct ArrayExpr : Expr {
    std::vector<std::unique_ptr<Expr>> elements;
};

struct ObjectExpr : Expr {
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> pairs;
};

struct LambdaExpr : Expr {
    std::vector<std::string> params;
    std::unique_ptr<ASTNode> body;
};

struct VarDeclStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    bool mutable_ = true;
};

struct AssignStmt : Stmt {
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBlock;
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<BlockStmt>>> elifBlocks;
    std::unique_ptr<BlockStmt> elseBlock;
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> init;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> update;
    std::unique_ptr<BlockStmt> body;
    std::string iterVar;
    std::unique_ptr<Expr> iterExpr;
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
};

struct FuncDeclStmt : Stmt {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<ASTNode> body;
};

struct ClassDeclStmt : Stmt {
    std::string name;
    std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expr>>> members;
    std::vector<std::unique_ptr<FuncDeclStmt>> methods;
};

struct ProgramNode : ASTNode {
    std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace mpp
