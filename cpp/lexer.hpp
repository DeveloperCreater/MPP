#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace mpp {

enum class TokenType {
    Number, String, Ident, True, False,
    Let, Const, Fn, If, Elif, Else, For, While, In, Return,
    Class, Pub, Priv, Self, New,
    Plus, Minus, Star, Slash, Percent,
    Eq, EqEq, Neq, Lt, Le, Gt, Ge, And, Or, Not, Arrow,
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Comma, Dot, Colon, Semicolon,
    Eof
};

struct Token {
    TokenType type;
    std::string value;
    int line, column;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_ = 0;
    int line_ = 1, column_ = 1;

    char current() const;
    char peek(size_t off = 1) const;
    char advance();
    void skipWhitespace();
    void skipComment();
    Token readNumber();
    Token readString(char quote);
    Token readIdent();
};

} // namespace mpp
