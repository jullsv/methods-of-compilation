#pragma once
#include <string>
#include <vector>

// --- Типы токенов для Лексера ---
enum TokenType {
    T_INT = 1, T_FLOAT, T_ID,
    T_KW_READ, T_KW_WRITE, T_KW_IF, T_KW_THEN, T_KW_ELSE, T_KW_BEGIN, T_KW_END,
    T_KW_WHILE, T_KW_DO, T_KW_SQRT, T_KW_EXP, T_KW_LOG,
    T_OP_ASSIGN, T_OP_ADD, T_OP_SUB, T_OP_MUL, T_OP_DIV,
    T_OP_LT, T_OP_GT, T_OP_LE, T_OP_GE, T_OP_EQ, T_OP_NE,
    T_SEP_LPAR, T_SEP_RPAR, T_SEP_LBRACK, T_SEP_RBRACK, T_SEP_SEMI,
    T_EOF, T_ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};

// --- Типы операций для ОПС (Интерпретатора) ---
enum OpType {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEG,
    OP_LT, OP_GT, OP_LE, OP_GE, OP_EQ, OP_NE,
    OP_PUSH_INT, OP_PUSH_FLOAT, OP_PUSH_VAR, OP_POP_VAR,
    OP_ARRAY_GET, OP_ARRAY_SET,
    OP_CALL_SQRT, OP_CALL_EXP, OP_CALL_LOG,
    OP_JMP, OP_JMP_FALSE,
    OP_READ, OP_PRINT,
    OP_END
};

struct Instruction {
    OpType op;
    std::string arg; // Имя переменной или число (как строка)
    int label;       // Адрес перехода

    Instruction(OpType _op) : op(_op), arg(""), label(-1) {}
    Instruction(OpType _op, const std::string& _arg) : op(_op), arg(_arg), label(-1) {}
    Instruction(OpType _op, int _label) : op(_op), arg(""), label(_label) {}
};