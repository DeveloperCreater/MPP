#pragma once

#include "lexer.hpp"
#include "ast.hpp"
#include <memory>
#include <stdexcept>

namespace mpp {

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& msg, const Token& t)
        : std::runtime_error("Line " + std::to_string(t.line) + ": " + msg) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<ProgramNode> parse();

private:
    std::vector<Token> tokens_;
    size_t pos_ = 0;

    const Token& current() const;
    const Token& advance();
    bool check(TokenType t) const;
    const Token& expect(TokenType t);
    bool check(TokenType a, TokenType b) const;

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<VarDeclStmt> parseVarDecl();
    std::unique_ptr<FuncDeclStmt> parseFuncDecl();
    std::unique_ptr<ClassDeclStmt> parseClassDecl();
    std::unique_ptr<IfStmt> parseIf();
    std::unique_ptr<ForStmt> parseFor();
    std::unique_ptr<WhileStmt> parseWhile();
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<BlockStmt> parseBlockRaw();

    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parseCall();
    std::unique_ptr<Expr> parsePrimary();

    bool isAssignable(Expr* e) const;
};

} // namespace mpp
