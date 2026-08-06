#include <kernel.h>
#include "yakka_lang.hpp"

namespace yakka {

void value_set_number(Value& v, long n) {
    v.type = Value::Type::Number;
    v.num = n;
    v.str[0] = 0;
}

void value_set_string(Value& v, const char* s) {
    v.type = Value::Type::String;
    v.num = 0;
    kstrncpy(v.str, s, 127);
    v.str[127] = 0;
}

bool value_truthy(const Value& v) {
    if (v.type == Value::Type::Number) {
        return v.num != 0;
    }

    if (v.type == Value::Type::String) {
        return v.str[0] != 0;
    }

    return false;
}

void value_to_string(const Value& v, char* out, size_t len) {
    if (v.type == Value::Type::String) {
        kstrncpy(out, v.str, len - 1);
        out[len - 1] = 0;
        return;
    }

    if (v.type == Value::Type::Number) {
        kitoa(v.num, out, 10);
        return;
    }

    out[0] = 0;
}

Parser::Parser(const char* source) : lexer(source) {
    current = lexer.next();
}

void Parser::advance() {
    current = lexer.next();
}

void Parser::fail() {
    had_error = true;
}

static int value_compare(const Value& a, const Value& b) {
    if (a.type == Value::Type::String || b.type == Value::Type::String) {
        char sa[128];
        char sb[128];
        value_to_string(a, sa, sizeof(sa));
        value_to_string(b, sb, sizeof(sb));
        return kstrcmp(sa, sb);
    }

    if (a.num < b.num) return -1;
    if (a.num > b.num) return 1;
    return 0;
}

static Value binary_value(const Value& a, TokenType op, const Value& b) {
    Value result;

    if (op == TokenType::Plus) {
        if (a.type == Value::Type::String || b.type == Value::Type::String) {
            char sa[128];
            char sb[128];
            value_to_string(a, sa, sizeof(sa));
            value_to_string(b, sb, sizeof(sb));
            kstrcat(sa, sb);
            value_set_string(result, sa);
        } else {
            value_set_number(result, a.num + b.num);
        }
        return result;
    }

    if (op == TokenType::Minus) {
        value_set_number(result, a.num - b.num);
        return result;
    }

    if (op == TokenType::Star) {
        value_set_number(result, a.num * b.num);
        return result;
    }

    if (op == TokenType::Slash) {
        if (b.num == 0) {
            value_set_number(result, 0);
        } else {
            value_set_number(result, a.num / b.num);
        }
        return result;
    }

    if (op == TokenType::Percent) {
        if (b.num == 0) {
            value_set_number(result, 0);
        } else {
            value_set_number(result, a.num % b.num);
        }
        return result;
    }

    int cmp = value_compare(a, b);

    switch (op) {
        case TokenType::Eq:
        case TokenType::Assign:
            value_set_number(result, cmp == 0);
            break;
        case TokenType::Neq:
            value_set_number(result, cmp != 0);
            break;
        case TokenType::Lt:
            value_set_number(result, cmp < 0);
            break;
        case TokenType::Gt:
            value_set_number(result, cmp > 0);
            break;
        case TokenType::LtEq:
            value_set_number(result, cmp <= 0);
            break;
        case TokenType::GtEq:
            value_set_number(result, cmp >= 0);
            break;
        default:
            value_set_number(result, 0);
            break;
    }

    return result;
}

Value Parser::parse_expression() {
    return parse_or();
}

Value Parser::parse_or() {
    Value left = parse_and();

    while (current.type == TokenType::Ident && kstrcmp(current.text, "or") == 0) {
        advance();
        Value right = parse_and();
        value_set_number(left, value_truthy(left) || value_truthy(right));
    }

    return left;
}

Value Parser::parse_and() {
    Value left = parse_comparison();

    while (current.type == TokenType::Ident && kstrcmp(current.text, "and") == 0) {
        advance();
        Value right = parse_comparison();
        value_set_number(left, value_truthy(left) && value_truthy(right));
    }

    return left;
}

Value Parser::parse_comparison() {
    Value left = parse_add();

    while (
        current.type == TokenType::Eq ||
        current.type == TokenType::Neq ||
        current.type == TokenType::Lt ||
        current.type == TokenType::Gt ||
        current.type == TokenType::LtEq ||
        current.type == TokenType::GtEq ||
        current.type == TokenType::Assign
    ) {
        TokenType op = current.type;
        advance();
        Value right = parse_add();
        left = binary_value(left, op, right);
    }

    return left;
}

Value Parser::parse_add() {
    Value left = parse_mul();

    while (current.type == TokenType::Plus || current.type == TokenType::Minus) {
        TokenType op = current.type;
        advance();
        Value right = parse_mul();
        left = binary_value(left, op, right);
    }

    return left;
}

Value Parser::parse_mul() {
    Value left = parse_unary();

    while (
        current.type == TokenType::Star ||
        current.type == TokenType::Slash ||
        current.type == TokenType::Percent
    ) {
        TokenType op = current.type;
        advance();
        Value right = parse_unary();
        left = binary_value(left, op, right);
    }

    return left;
}

Value Parser::parse_unary() {
    if (current.type == TokenType::Minus) {
        advance();
        Value v = parse_primary();

        if (v.type == Value::Type::Number) {
            value_set_number(v, -v.num);
        }

        return v;
    }

    return parse_primary();
}

Value Parser::parse_primary() {
    if (current.type == TokenType::Number) {
        Value v;
        value_set_number(v, current.num);
        advance();
        return v;
    }

    if (current.type == TokenType::String) {
        Value v;
        value_set_string(v, current.text);
        advance();
        return v;
    }

    if (current.type == TokenType::LParen) {
        advance();
        Value v = parse_expression();

        if (current.type == TokenType::RParen) {
            advance();
        } else {
            fail();
        }

        return v;
    }

    if (current.type == TokenType::Ident) {
        char name[128];
        kstrncpy(name, current.text, 127);
        name[127] = 0;
        advance();

        if (current.type == TokenType::LParen) {
            advance();

            Value args[8];
            int argc = 0;

            while (current.type != TokenType::RParen && current.type != TokenType::End) {
                if (argc < 8) {
                    args[argc++] = parse_expression();
                } else {
                    parse_expression();
                }

                if (current.type == TokenType::Comma) {
                    advance();
                } else {
                    break;
                }
            }

            if (current.type == TokenType::RParen) {
                advance();
            } else {
                fail();
            }

            return call_builtin(name, args, argc);
        }

        Value v;
        if (!runtime_get_variable(name, v)) {
            value_set_number(v, 0);
        }

        return v;
    }

    fail();
    advance();

    Value v;
    value_set_number(v, 0);
    return v;
}

}
