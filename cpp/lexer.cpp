#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace mpp {

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"let", TokenType::Let}, {"const", TokenType::Const}, {"fn", TokenType::Fn},
    {"if", TokenType::If}, {"elif", TokenType::Elif}, {"else", TokenType::Else},
    {"for", TokenType::For}, {"while", TokenType::While}, {"in", TokenType::In},
    {"return", TokenType::Return}, {"class", TokenType::Class},
    {"pub", TokenType::Pub}, {"priv", TokenType::Priv}, {"self", TokenType::Self},
    {"new", TokenType::New}, {"true", TokenType::True}, {"false", TokenType::False},
};

Lexer::Lexer(const std::string& source) : source_(source) {}

char Lexer::current() const {
    return pos_ >= source_.size() ? '\0' : source_[pos_];
}

char Lexer::peek(size_t off) const {
    size_t p = pos_ + off;
    return p >= source_.size() ? '\0' : source_[p];
}

char Lexer::advance() {
    if (pos_ >= source_.size()) return '\0';
    char ch = source_[pos_++];
    if (ch == '\n') { line_++; column_ = 1; }
    else column_++;
    return ch;
}

void Lexer::skipWhitespace() {
    while (current() == ' ' || current() == '\t' || current() == '\n' || current() == '\r')
        advance();
}

void Lexer::skipComment() {
    if (current() == '/' && peek() == '/')
        while (current() != '\n' && current() != '\0') advance();
}

Token Lexer::readNumber() {
    int sl = line_, sc = column_;
    std::string s;
    while (std::isdigit(current()) || current() == '.')
        s += advance();
    Token t;
    t.type = TokenType::Number;
    t.value = s;
    t.line = sl; t.column = sc;
    return t;
}

Token Lexer::readString(char quote) {
    int sl = line_, sc = column_;
    advance();
    std::string s;
    while (current() != quote && current() != '\0') {
        if (current() == '\\') {
            advance();
            char e = advance();
            if (e == 'n') s += '\n';
            else if (e == 't') s += '\t';
            else if (e == '"' || e == '\'') s += e;
            else s += e;
        } else s += advance();
    }
    if (current() == quote) advance();
    Token t;
    t.type = TokenType::String;
    t.value = s;
    t.line = sl; t.column = sc;
    return t;
}

Token Lexer::readIdent() {
    int sl = line_, sc = column_;
    std::string s;
    while (std::isalnum(static_cast<unsigned char>(current())) || current() == '_')
        s += advance();
    Token t;
    auto it = KEYWORDS.find(s);
    t.type = it != KEYWORDS.end() ? it->second : TokenType::Ident;
    t.value = s;
    t.line = sl; t.column = sc;
    return t;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos_ < source_.size()) {
        skipWhitespace();
        if (pos_ >= source_.size()) break;
        skipComment();
        if (pos_ >= source_.size()) break;
        char ch = current();
        int sl = line_, sc = column_;

        if (ch == '=' && peek() == '=') { advance(); advance(); tokens.push_back({TokenType::EqEq, "==", sl, sc}); continue; }
        if (ch == '!' && peek() == '=') { advance(); advance(); tokens.push_back({TokenType::Neq, "!=", sl, sc}); continue; }
        if (ch == '<' && peek() == '=') { advance(); advance(); tokens.push_back({TokenType::Le, "<=", sl, sc}); continue; }
        if (ch == '>' && peek() == '=') { advance(); advance(); tokens.push_back({TokenType::Ge, ">=", sl, sc}); continue; }
        if (ch == '-' && peek() == '>') { advance(); advance(); tokens.push_back({TokenType::Arrow, "=>", sl, sc}); continue; }
        if (ch == '=' && peek() == '>') { advance(); advance(); tokens.push_back({TokenType::Arrow, "=>", sl, sc}); continue; }
        if (ch == '&' && peek() == '&') { advance(); advance(); tokens.push_back({TokenType::And, "&&", sl, sc}); continue; }
        if (ch == '|' && peek() == '|') { advance(); advance(); tokens.push_back({TokenType::Or, "||", sl, sc}); continue; }

        if (ch == '+') { advance(); tokens.push_back({TokenType::Plus, "+", sl, sc}); }
        else if (ch == '-') { advance(); tokens.push_back({TokenType::Minus, "-", sl, sc}); }
        else if (ch == '*') { advance(); tokens.push_back({TokenType::Star, "*", sl, sc}); }
        else if (ch == '/') { advance(); tokens.push_back({TokenType::Slash, "/", sl, sc}); }
        else if (ch == '%') { advance(); tokens.push_back({TokenType::Percent, "%", sl, sc}); }
        else if (ch == '=') { advance(); tokens.push_back({TokenType::Eq, "=", sl, sc}); }
        else if (ch == '<') { advance(); tokens.push_back({TokenType::Lt, "<", sl, sc}); }
        else if (ch == '>') { advance(); tokens.push_back({TokenType::Gt, ">", sl, sc}); }
        else if (ch == '!') { advance(); tokens.push_back({TokenType::Not, "!", sl, sc}); }
        else if (ch == '(') { advance(); tokens.push_back({TokenType::LParen, "(", sl, sc}); }
        else if (ch == ')') { advance(); tokens.push_back({TokenType::RParen, ")", sl, sc}); }
        else if (ch == '{') { advance(); tokens.push_back({TokenType::LBrace, "{", sl, sc}); }
        else if (ch == '}') { advance(); tokens.push_back({TokenType::RBrace, "}", sl, sc}); }
        else if (ch == '[') { advance(); tokens.push_back({TokenType::LBracket, "[", sl, sc}); }
        else if (ch == ']') { advance(); tokens.push_back({TokenType::RBracket, "]", sl, sc}); }
        else if (ch == ',') { advance(); tokens.push_back({TokenType::Comma, ",", sl, sc}); }
        else if (ch == '.') { advance(); tokens.push_back({TokenType::Dot, ".", sl, sc}); }
        else if (ch == ':') { advance(); tokens.push_back({TokenType::Colon, ":", sl, sc}); }
        else if (ch == ';') { advance(); tokens.push_back({TokenType::Semicolon, ";", sl, sc}); }
        else if (ch == '"' || ch == '\'') tokens.push_back(readString(ch));
        else if (std::isdigit(static_cast<unsigned char>(ch))) tokens.push_back(readNumber());
        else if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') tokens.push_back(readIdent());
        else advance();
    }
    tokens.push_back({TokenType::Eof, "", line_, column_});
    return tokens;
}

} // namespace mpp
