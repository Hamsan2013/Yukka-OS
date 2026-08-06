#include <kernel.h>
#include "yakka_lang.hpp"

namespace yakka {

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_ident_char(char c) {
    return is_alpha(c) || is_digit(c);
}

Lexer::Lexer(const char* source) : src(source), pos(0) {
}

char Lexer::peek() {
    return src[pos];
}

char Lexer::advance() {
    return src[pos++];
}

void Lexer::skip_spaces() {
    while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\r') {
        pos++;
    }
}

Token Lexer::make(TokenType type, const char* text) {
    Token token;
    token.type = type;
    token.num = 0;
    kstrncpy(token.text, text, 127);
    token.text[127] = 0;
    return token;
}

Token Lexer::number() {
    char buf[32];
    int len = 0;

    while (is_digit(peek()) && len < 31) {
        buf[len++] = advance();
    }

    buf[len] = 0;

    Token token = make(TokenType::Number, buf);
    token.num = katoi(buf);
    return token;
}

Token Lexer::string() {
    advance();

    char buf[128];
    int len = 0;

    while (peek() && peek() != '"' && len < 127) {
        buf[len++] = advance();
    }

    if (peek() == '"') {
        advance();
    }

    buf[len] = 0;
    return make(TokenType::String, buf);
}

Token Lexer::identifier() {
    char buf[128];
    int len = 0;

    while (is_ident_char(peek()) && len < 127) {
        buf[len++] = advance();
    }

    buf[len] = 0;

    if (kstrcmp(buf, "let") == 0) return make(TokenType::KeywordLet, buf);
    if (kstrcmp(buf, "print") == 0) return make(TokenType::KeywordPrint, buf);
    if (kstrcmp(buf, "if") == 0) return make(TokenType::KeywordIf, buf);
    if (kstrcmp(buf, "repeat") == 0) return make(TokenType::KeywordRepeat, buf);
    if (kstrcmp(buf, "function") == 0) return make(TokenType::KeywordFunction, buf);
    if (kstrcmp(buf, "end") == 0) return make(TokenType::KeywordEnd, buf);

    return make(TokenType::Ident, buf);
}

Token Lexer::next() {
    skip_spaces();

    if (!peek()) {
        return make(TokenType::End, "");
    }

    char c = peek();

    if (is_digit(c)) {
        return number();
    }

    if (c == '"') {
        return string();
    }

    if (is_alpha(c)) {
        return identifier();
    }

    advance();

    switch (c) {
        case '+': return make(TokenType::Plus, "+");
        case '-': return make(TokenType::Minus, "-");
        case '*': return make(TokenType::Star, "*");
        case '/': return make(TokenType::Slash, "/");
        case '%': return make(TokenType::Percent, "%");
        case '(': return make(TokenType::LParen, "(");
        case ')': return make(TokenType::RParen, ")");
        case ',': return make(TokenType::Comma, ",");
        case '=':
            if (peek() == '=') {
                advance();
                return make(TokenType::Eq, "==");
            }
            return make(TokenType::Assign, "=");
        case '!':
            if (peek() == '=') {
                advance();
                return make(TokenType::Neq, "!=");
            }
            return make(TokenType::Unknown, "!");
        case '<':
            if (peek() == '=') {
                advance();
                return make(TokenType::LtEq, "<=");
            }
            return make(TokenType::Lt, "<");
        case '>':
            if (peek() == '=') {
                advance();
                return make(TokenType::GtEq, ">=");
            }
            return make(TokenType::Gt, ">");
    }

    return make(TokenType::Unknown, "?");
}

}
