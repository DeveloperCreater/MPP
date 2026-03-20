#include "parser.hpp"
#include <algorithm>

namespace mpp {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::current() const {
    return pos_ >= tokens_.size() ? tokens_.back() : tokens_[pos_];
}

const Token& Parser::advance() {
    const Token& t = current();
    if (pos_ < tokens_.size()) pos_++;
    return t;
}

bool Parser::check(TokenType t) const { return current().type == t; }
bool Parser::check(TokenType a, TokenType b) const {
    TokenType c = current().type;
    return c == a || c == b;
}

const Token& Parser::expect(TokenType t) {
    if (current().type != t)
        throw ParseError("Unexpected token", current());
    return advance();
}

bool Parser::isAssignable(Expr* e) const {
    return dynamic_cast<IdentExpr*>(e) || dynamic_cast<IndexExpr*>(e) || dynamic_cast<MemberExpr*>(e);
}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto prog = std::make_unique<ProgramNode>();
    while (!check(TokenType::Eof)) {
        auto stmt = parseStatement();
        if (stmt) prog->statements.push_back(std::move(stmt));
        if (check(TokenType::Semicolon)) advance();
    }
    return prog;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::Let) || check(TokenType::Const))
        return parseVarDecl();
    if (check(TokenType::Fn)) return parseFuncDecl();
    if (check(TokenType::Class)) return parseClassDecl();
    if (check(TokenType::If)) return parseIf();
    if (check(TokenType::For)) return parseFor();
    if (check(TokenType::While)) return parseWhile();
    if (check(TokenType::Return)) {
        advance();
        std::unique_ptr<Expr> val;
        if (!check(TokenType::Semicolon) && !check(TokenType::RBrace) && !check(TokenType::Eof))
            val = parseExpr();
        return std::make_unique<ReturnStmt>(std::move(val));
    }
    if (check(TokenType::LBrace)) return parseBlock();

    auto expr = parseExpr();
    if (expr && check(TokenType::Eq) && isAssignable(expr.get())) {
        advance();
        auto target = std::move(expr);
        auto value = parseExpr();
        auto a = std::make_unique<AssignStmt>();
        a->target = std::move(target);
        a->value = std::move(value);
        return a;
    }
    if (expr) return std::make_unique<ExprStmt>(std::move(expr));
    return nullptr;
}

std::unique_ptr<VarDeclStmt> Parser::parseVarDecl() {
    advance();
    std::string name = expect(TokenType::Ident).value;
    expect(TokenType::Eq);
    auto value = parseExpr();
    auto v = std::make_unique<VarDeclStmt>();
    v->name = name;
    v->value = std::move(value);
    return v;
}

std::unique_ptr<FuncDeclStmt> Parser::parseFuncDecl() {
    advance();
    std::string name = expect(TokenType::Ident).value;
    expect(TokenType::LParen);
    std::vector<std::string> params;
    while (!check(TokenType::RParen)) {
        if (check(TokenType::Ident) || check(TokenType::Self))
            params.push_back(advance().value);
        if (check(TokenType::Colon)) { advance(); expect(TokenType::Ident); }
        if (!check(TokenType::RParen)) expect(TokenType::Comma);
    }
    expect(TokenType::RParen);
    if (check(TokenType::Colon)) { advance(); expect(TokenType::Ident); }
    std::unique_ptr<ASTNode> body;
    if (check(TokenType::Arrow)) {
        advance();
        body = std::make_unique<ExprStmt>(parseExpr());
    } else {
        body = parseBlockRaw();
    }
    auto f = std::make_unique<FuncDeclStmt>();
    f->name = name;
    f->params = std::move(params);
    f->body = std::move(body);
    return f;
}

std::unique_ptr<ClassDeclStmt> Parser::parseClassDecl() {
    advance();
    std::string name = expect(TokenType::Ident).value;
    expect(TokenType::LBrace);
    auto c = std::make_unique<ClassDeclStmt>();
    c->name = name;
    while (!check(TokenType::RBrace) && !check(TokenType::Eof)) {
        if (check(TokenType::Pub) || check(TokenType::Priv)) {
            std::string vis = advance().value;
            std::string mname = expect(TokenType::Ident).value;
            std::unique_ptr<Expr> def;
            if (check(TokenType::Eq)) { advance(); def = parseExpr(); }
            c->members.emplace_back(vis, mname, std::move(def));
        } else if (check(TokenType::Fn)) {
            auto stmt = parseFuncDecl();
            c->methods.push_back(std::unique_ptr<FuncDeclStmt>(static_cast<FuncDeclStmt*>(stmt.release())));
        } else advance();
    }
    expect(TokenType::RBrace);
    return c;
}

std::unique_ptr<IfStmt> Parser::parseIf() {
    advance();
    auto cond = parseExpr();
    if (check(TokenType::LParen)) { advance(); expect(TokenType::RParen); }
    auto thenB = parseBlock();
    auto i = std::make_unique<IfStmt>();
    i->condition = std::move(cond);
    i->thenBlock = std::move(thenB);
    while (check(TokenType::Elif)) {
        advance();
        auto ec = parseExpr();
        auto eb = parseBlock();
        i->elifBlocks.emplace_back(std::move(ec), std::move(eb));
    }
    if (check(TokenType::Else)) {
        advance();
        i->elseBlock = parseBlock();
    }
    return i;
}

std::unique_ptr<ForStmt> Parser::parseFor() {
    advance();
    expect(TokenType::LParen);
    auto f = std::make_unique<ForStmt>();
    if (check(TokenType::Let)) {
        advance();
        std::string iterVar = expect(TokenType::Ident).value;
        if (check(TokenType::In)) {
            advance();
            f->iterVar = iterVar;
            f->iterExpr = parseExpr();
            expect(TokenType::RParen);
            f->body = parseBlock();
            return f;
        }
        expect(TokenType::Eq);
        auto v = std::make_unique<VarDeclStmt>();
        v->name = iterVar;
        v->value = parseExpr();
        f->init = std::move(v);
    } else if (check(TokenType::Ident)) {
        std::string n = advance().value;
        if (check(TokenType::In)) {
            advance();
            f->iterVar = n;
            f->iterExpr = parseExpr();
            expect(TokenType::RParen);
            f->body = parseBlock();
            return f;
        }
        if (check(TokenType::Eq)) {
            advance();
            auto a = std::make_unique<AssignStmt>();
            a->target = std::make_unique<IdentExpr>();
            static_cast<IdentExpr*>(a->target.get())->name = n;
            a->value = parseExpr();
            f->init = std::move(a);
        } else {
            auto e = std::make_unique<ExprStmt>();
            e->expr = std::make_unique<IdentExpr>();
            static_cast<IdentExpr*>(e->expr.get())->name = n;
            f->init = std::move(e);
        }
    } else if (!check(TokenType::Semicolon)) {
        f->init = parseStatement();
    }
    if (check(TokenType::Semicolon)) advance();
    if (!check(TokenType::Semicolon)) f->condition = parseExpr();
    expect(TokenType::Semicolon);
    if (!check(TokenType::RParen)) f->update = parseStatement();
    expect(TokenType::RParen);
    f->body = parseBlock();
    return f;
}

std::unique_ptr<WhileStmt> Parser::parseWhile() {
    advance();
    auto cond = parseExpr();
    if (check(TokenType::LParen)) { advance(); expect(TokenType::RParen); }
    auto w = std::make_unique<WhileStmt>();
    w->condition = std::move(cond);
    w->body = parseBlock();
    return w;
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    expect(TokenType::LBrace);
    return parseBlockRaw();
}

std::unique_ptr<BlockStmt> Parser::parseBlockRaw() {
    auto b = std::make_unique<BlockStmt>();
    while (!check(TokenType::RBrace) && !check(TokenType::Eof)) {
        auto s = parseStatement();
        if (s) b->statements.push_back(std::move(s));
        if (check(TokenType::Semicolon)) advance();
    }
    expect(TokenType::RBrace);
    return b;
}

std::unique_ptr<Expr> Parser::parseExpr() { return parseOr(); }

std::unique_ptr<Expr> Parser::parseOr() {
    auto left = parseAnd();
    while (check(TokenType::Or)) {
        advance();
        auto b = std::make_unique<BinaryExpr>();
        b->left = std::move(left);
        b->op = "||";
        b->right = parseAnd();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAnd() {
    auto left = parseEquality();
    while (check(TokenType::And)) {
        advance();
        auto b = std::make_unique<BinaryExpr>();
        b->left = std::move(left);
        b->op = "&&";
        b->right = parseEquality();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseComparison();
    while (check(TokenType::EqEq) || check(TokenType::Neq)) {
        std::string op = advance().value;
        auto b = std::make_unique<BinaryExpr>();
        b->left = std::move(left);
        b->op = op;
        b->right = parseComparison();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseTerm();
    while (check(TokenType::Lt) || check(TokenType::Le) || check(TokenType::Gt) || check(TokenType::Ge)) {
        std::string op = advance().value;
        auto b = std::make_unique<BinaryExpr>();
        b->left = std::move(left);
        b->op = op;
        b->right = parseTerm();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        std::string op = advance().value;
        auto b = std::make_unique<BinaryExpr>();
        b->left = std::move(left);
        b->op = op;
        b->right = parseFactor();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto left = parseUnary();
    while (check(TokenType::Star) || check(TokenType::Slash) || check(TokenType::Percent)) {
        std::string op = advance().value;
        auto b = std::make_unique<BinaryExpr>();
        b->left = std::move(left);
        b->op = op;
        b->right = parseUnary();
        left = std::move(b);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (check(TokenType::Minus) || check(TokenType::Not)) {
        std::string op = advance().value;
        auto u = std::make_unique<UnaryExpr>();
        u->op = op;
        u->right = parseUnary();
        return u;
    }
    return parseCall();
}

std::unique_ptr<Expr> Parser::parseCall() {
    auto base = parsePrimary();
    while (true) {
        if (check(TokenType::LParen)) {
            advance();
            auto c = std::make_unique<CallExpr>();
            c->callee = std::move(base);
            while (!check(TokenType::RParen)) {
                c->args.push_back(parseExpr());
                if (!check(TokenType::RParen)) expect(TokenType::Comma);
            }
            expect(TokenType::RParen);
            base = std::move(c);
        } else if (check(TokenType::LBracket)) {
            advance();
            auto idx = std::make_unique<IndexExpr>();
            idx->obj = std::move(base);
            idx->index = parseExpr();
            expect(TokenType::RBracket);
            base = std::move(idx);
        } else if (check(TokenType::Dot)) {
            advance();
            auto m = std::make_unique<MemberExpr>();
            m->obj = std::move(base);
            m->member = expect(TokenType::Ident).value;
            base = std::move(m);
        } else break;
    }
    return base;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::New)) {
        advance();
        std::string cls = expect(TokenType::Ident).value;
        expect(TokenType::LParen);
        auto c = std::make_unique<CallExpr>();
        c->callee = std::make_unique<IdentExpr>();
        static_cast<IdentExpr*>(c->callee.get())->name = cls;
        while (!check(TokenType::RParen)) {
            c->args.push_back(parseExpr());
            if (!check(TokenType::RParen)) expect(TokenType::Comma);
        }
        expect(TokenType::RParen);
        return c;
    }
    if (check(TokenType::Number)) {
        auto n = std::make_unique<NumberExpr>();
        n->value = std::stod(advance().value);
        return n;
    }
    if (check(TokenType::String)) {
        auto s = std::make_unique<StringExpr>();
        s->value = advance().value;
        return s;
    }
    if (check(TokenType::True)) { advance(); auto b = std::make_unique<BoolExpr>(); b->value = true; return b; }
    if (check(TokenType::False)) { advance(); auto b = std::make_unique<BoolExpr>(); b->value = false; return b; }
    if (check(TokenType::Ident) || check(TokenType::Self)) {
        auto i = std::make_unique<IdentExpr>();
        i->name = advance().value;
        return i;
    }
    if (check(TokenType::LParen)) {
        advance();
        auto e = parseExpr();
        expect(TokenType::RParen);
        return e;
    }
    if (check(TokenType::LBracket)) {
        advance();
        auto a = std::make_unique<ArrayExpr>();
        while (!check(TokenType::RBracket)) {
            a->elements.push_back(parseExpr());
            if (!check(TokenType::RBracket)) expect(TokenType::Comma);
        }
        expect(TokenType::RBracket);
        return a;
    }
    if (check(TokenType::LBrace)) {
        advance();
        auto o = std::make_unique<ObjectExpr>();
        while (!check(TokenType::RBrace)) {
            std::string k = expect(TokenType::Ident).value;
            expect(TokenType::Colon);
            o->pairs.emplace_back(k, parseExpr());
            if (!check(TokenType::RBrace)) expect(TokenType::Comma);
        }
        expect(TokenType::RBrace);
        return o;
    }
    if (check(TokenType::Fn)) {
        advance();
        expect(TokenType::LParen);
        std::vector<std::string> params;
        while (!check(TokenType::RParen)) {
            if (check(TokenType::Ident) || check(TokenType::Self)) params.push_back(advance().value);
            if (check(TokenType::Colon)) { advance(); expect(TokenType::Ident); }
            if (!check(TokenType::RParen)) expect(TokenType::Comma);
        }
        expect(TokenType::RParen);
        std::unique_ptr<ASTNode> body;
        if (check(TokenType::Arrow)) { advance(); body = std::make_unique<ExprStmt>(parseExpr()); }
        else body = parseBlockRaw();
        auto l = std::make_unique<LambdaExpr>();
        l->params = std::move(params);
        l->body = std::move(body);
        return l;
    }
    throw ParseError("Unexpected token", current());
}

} // namespace mpp
