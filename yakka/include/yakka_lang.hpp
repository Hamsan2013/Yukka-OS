#pragma once
#include <kernel.h>

namespace yakka {

enum class TokenType {
    End,
    Number,
    String,
    Ident,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Assign,
    Eq,
    Neq,
    Lt,
    Gt,
    LtEq,
    GtEq,
    LParen,
    RParen,
    Comma,
    KeywordLet,
    KeywordPrint,
    KeywordIf,
    KeywordRepeat,
    KeywordFunction,
    KeywordEnd,
    Unknown
};

struct Token {
    TokenType type;
    char text[128];
    long num;
};

struct Value {
    enum class Type {
        None,
        Number,
        String
    };

    Type type;
    long num;
    char str[128];

    Value() : type(Type::None), num(0) {
        str[0] = 0;
    }
};

void value_set_number(Value& v, long n);
void value_set_string(Value& v, const char* s);
bool value_truthy(const Value& v);
void value_to_string(const Value& v, char* out, size_t len);

class Lexer {
public:
    explicit Lexer(const char* source);
    Token next();

private:
    const char* src;
    int pos;

    char peek();
    char advance();
    void skip_spaces();
    Token make(TokenType type, const char* text);
    Token number();
    Token string();
    Token identifier();
};

class Parser {
public:
    explicit Parser(const char* source);
    Value parse_expression();
    bool had_error = false;

private:
    Lexer lexer;
    Token current;

    void advance();
    void fail();
    Value parse_or();
    Value parse_and();
    Value parse_comparison();
    Value parse_add();
    Value parse_mul();
    Value parse_unary();
    Value parse_primary();
};

int execute_program(const char* lines[], int count);

bool runtime_get_variable(const char* name, Value& out);
void runtime_set_variable(const char* name, const Value& value);
bool runtime_call_function(const char* name);
void runtime_define_function(const char* name, const char* lines[], int count);
void runtime_print_value(const Value& value);
void runtime_start_capture();
void runtime_stop_capture();
const char* runtime_captured();
void runtime_reset();

Value call_builtin(const char* name, Value* args, int argc);

}
