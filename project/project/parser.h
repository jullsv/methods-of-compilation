#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <stdexcept>
#include "ops.h"
#include "lexer.h"

class Parser {
private:
    Lexer& lexer;
    Token currentToken;
    std::vector<Instruction> code;

    void nextToken() {
        currentToken = lexer.nextToken();
        if (currentToken.type == T_ERROR) {
            throw std::runtime_error("Lexical Error at line " + std::to_string(currentToken.line) +
                ", col " + std::to_string(currentToken.col) + ": Unexpected '" + currentToken.value + "'");
        }
    }

    void match(TokenType expected) {
        if (currentToken.type == expected) {
            nextToken();
        }
        else {
            throw std::runtime_error("Syntax Error at line " + std::to_string(currentToken.line) +
                ", col " + std::to_string(currentToken.col) +
                ": Expected token type " + std::to_string(expected) +
                ", but got " + std::to_string(currentToken.type));
        }
    }

public:
    Parser(Lexer& l) : lexer(l) {
        nextToken();
    }

    std::vector<Instruction> parse() {
        parseStatementList();
        code.push_back(Instruction(OP_END));
        return code;
    }

private:
    void parseStatementList() {
        if (currentToken.type == T_EOF || currentToken.type == T_KW_END) {
            return;
        }

        if (currentToken.type == T_ID) {
            std::string varName = currentToken.value;
            nextToken();

            if (currentToken.type == T_OP_ASSIGN) {
                match(T_OP_ASSIGN);
                parseExpression();
                code.push_back(Instruction(OP_POP_VAR, varName));
                match(T_SEP_SEMI);
                parseStatementList();
            }
            else if (currentToken.type == T_SEP_LBRACK) {
                nextToken(); // [
                parseExpression(); // Index
                match(T_SEP_RBRACK); // ]
                match(T_OP_ASSIGN);
                parseExpression(); // Value
                code.push_back(Instruction(OP_ARRAY_SET, varName));
                match(T_SEP_SEMI);
                parseStatementList();
            }
            else {
                throw std::runtime_error("Syntax Error: Expected '=' or '[' after ID");
            }
        }
        else if (currentToken.type == T_KW_IF) {
            parseIfStatement();
            parseStatementList();
        }
        else if (currentToken.type == T_KW_WHILE) {
            parseWhileStatement();
            parseStatementList();
        }
        else if (currentToken.type == T_KW_READ) {
            nextToken(); // Пропускаем READ

            if (currentToken.type != T_ID) {
                throw std::runtime_error("Expected ID after READ");
            }

            std::string varName = currentToken.value;
            nextToken(); // Пропускаем имя переменной/массива

            // Проверяем, это массив или простая переменная?
            if (currentToken.type == T_SEP_LBRACK) {
                // --- Случай READ arr[i] ---
                nextToken(); // [
                parseExpression(); // Индекс
                match(T_SEP_RBRACK); // ]

                // Генерируем код: READ -> Stack, затем ARRAY_SET
                code.push_back(Instruction(OP_READ));

                code.push_back(Instruction(OP_ARRAY_SET, varName));
            }
            else {
                // --- Случай READ x ---
                code.push_back(Instruction(OP_READ));
                code.push_back(Instruction(OP_POP_VAR, varName));
            }

            match(T_SEP_SEMI);
            parseStatementList();
        }
        else if (currentToken.type == T_KW_WRITE) {
            nextToken();
            parseExpression();
            code.push_back(Instruction(OP_PRINT));
            match(T_SEP_SEMI);
            parseStatementList();
        }
        else if (currentToken.type == T_KW_BEGIN) {
            nextToken();
            parseStatementList();
            match(T_KW_END);
            match(T_SEP_SEMI);
            parseStatementList();
        }
        else {
            throw std::runtime_error("Syntax Error: Unexpected token in StatementList");
        }
    }

    void parseIfStatement() {
        match(T_KW_IF);
        parseExpression();

        int jmpFalseIdx = code.size();
        code.push_back(Instruction(OP_JMP_FALSE, -1));

        match(T_KW_THEN);
        parseStatementList();

        int jmpEndIdx = -1;
        if (currentToken.type == T_KW_ELSE) {
            jmpEndIdx = code.size();
            code.push_back(Instruction(OP_JMP, -1));
            code[jmpFalseIdx].label = code.size();
            nextToken(); // ELSE
            parseStatementList();
        }
        else {
            code[jmpFalseIdx].label = code.size();
        }

        match(T_KW_END);
        match(T_SEP_SEMI);

        if (jmpEndIdx != -1) {
            code[jmpEndIdx].label = code.size();
        }
    }

    void parseWhileStatement() {
        int startLabel = code.size();

        match(T_KW_WHILE);
        parseExpression();

        int jmpFalseIdx = code.size();
        code.push_back(Instruction(OP_JMP_FALSE, -1));

        match(T_KW_DO);
        parseStatementList();

        code.push_back(Instruction(OP_JMP, startLabel));
        code[jmpFalseIdx].label = code.size();

        match(T_KW_END);
        match(T_SEP_SEMI);
    }

    void parseExpression() {
        if (currentToken.type == T_INT) {
            code.push_back(Instruction(OP_PUSH_INT, currentToken.value));
            nextToken();
            parseETail();
        }
        else if (currentToken.type == T_FLOAT) {
            code.push_back(Instruction(OP_PUSH_FLOAT, currentToken.value));
            nextToken();
            parseETail();
        }
        else if (currentToken.type == T_ID) {
            std::string varName = currentToken.value;
            nextToken();
            parseE_ID_Tail(varName);
        }
        else if (currentToken.type == T_SEP_LPAR) {
            nextToken();
            parseExpression();
            match(T_SEP_RPAR);
            parseETail();
        }
        else if (currentToken.type == T_KW_SQRT || currentToken.type == T_KW_EXP || currentToken.type == T_KW_LOG) {
            OpType funcOp;
            if (currentToken.type == T_KW_SQRT) funcOp = OP_CALL_SQRT;
            else if (currentToken.type == T_KW_EXP) funcOp = OP_CALL_EXP;
            else funcOp = OP_CALL_LOG;

            nextToken();
            match(T_SEP_LPAR);
            parseExpression();
            match(T_SEP_RPAR);
            code.push_back(Instruction(funcOp));
            parseETail();
        }
        else {
            throw std::runtime_error("Syntax Error in Expression");
        }
    }

    void parseETail() {
        if (currentToken.type == T_OP_ADD) {
            nextToken();
            parseM();
            code.push_back(Instruction(OP_ADD));
            parseETail();
        }
        else if (currentToken.type == T_OP_SUB) {
            nextToken();
            parseM();
            code.push_back(Instruction(OP_SUB));
            parseETail();
        }
        else if (isCompOp(currentToken.type)) {
            parseCompTail();
        }
    }

    bool isCompOp(TokenType t) {
        return t == T_OP_LT || t == T_OP_GT || t == T_OP_LE ||
            t == T_OP_GE || t == T_OP_EQ || t == T_OP_NE;
    }

    void parseCompTail() {
        OpType op;
        switch (currentToken.type) {
        case T_OP_LT: op = OP_LT; break;
        case T_OP_GT: op = OP_GT; break;
        case T_OP_LE: op = OP_LE; break;
        case T_OP_GE: op = OP_GE; break;
        case T_OP_EQ: op = OP_EQ; break;
        case T_OP_NE: op = OP_NE; break;
        default: return;
        }
        nextToken();
        parseM();
        code.push_back(Instruction(op));
        parseCompTail();
    }

    void parseM() {
        parseU();
        parseMTail();
    }

    void parseMTail() {
        if (currentToken.type == T_OP_MUL) {
            nextToken();
            parseU();
            code.push_back(Instruction(OP_MUL));
            parseMTail();
        }
        else if (currentToken.type == T_OP_DIV) {
            nextToken();
            parseU();
            code.push_back(Instruction(OP_DIV));
            parseMTail();
        }
    }

    void parseU() {
        if (currentToken.type == T_OP_SUB) {
            nextToken();
            parseU();
            code.push_back(Instruction(OP_NEG));
        }
        else if (currentToken.type == T_INT) {
            code.push_back(Instruction(OP_PUSH_INT, currentToken.value));
            nextToken();
        }
        else if (currentToken.type == T_FLOAT) {
            code.push_back(Instruction(OP_PUSH_FLOAT, currentToken.value));
            nextToken();
        }
        else if (currentToken.type == T_ID) {
            std::string varName = currentToken.value;
            nextToken();
            parseU_ID_Tail(varName);
        }
        else if (currentToken.type == T_SEP_LPAR) {
            nextToken();
            parseExpression();
            match(T_SEP_RPAR);
        }
        else if (currentToken.type == T_KW_SQRT || currentToken.type == T_KW_EXP || currentToken.type == T_KW_LOG) {
            OpType funcOp;
            if (currentToken.type == T_KW_SQRT) funcOp = OP_CALL_SQRT;
            else if (currentToken.type == T_KW_EXP) funcOp = OP_CALL_EXP;
            else funcOp = OP_CALL_LOG;

            nextToken();
            match(T_SEP_LPAR);
            parseExpression();
            match(T_SEP_RPAR);
            code.push_back(Instruction(funcOp));
        }
        else {
            throw std::runtime_error("Syntax Error in Unary");
        }
    }

    void parseE_ID_Tail(const std::string& varName) {
        if (currentToken.type == T_SEP_LBRACK) {
            nextToken();
            parseExpression();
            match(T_SEP_RBRACK);
            code.push_back(Instruction(OP_ARRAY_GET, varName));
            parseETail();
        }
        else {
            code.push_back(Instruction(OP_PUSH_VAR, varName));
            parseETail();
        }
    }

    void parseU_ID_Tail(const std::string& varName) {
        if (currentToken.type == T_SEP_LBRACK) {
            nextToken();
            parseExpression();
            match(T_SEP_RBRACK);
            code.push_back(Instruction(OP_ARRAY_GET, varName));
        }
        else {
            code.push_back(Instruction(OP_PUSH_VAR, varName));
        }
    }
};