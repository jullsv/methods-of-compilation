#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <iomanip>
#include "ops.h" // Подключаем общие типы

// --- Классы символов и Состояния автомата ---
enum CharClass {
    C_LET = 0, C_DIG, C_DOT, C_SPACE, C_OPCHAR, C_SEP, C_OTHER, C_EOF
};

enum State {
    S_START = 0, S_ID, S_NUM, S_FLOAT, S_OP, S_SEP, S_ERROR
};

class Lexer {
private:
    std::string source;
    size_t pos;
    int line;
    int col;
    int transitionTable[7][8];
    std::map<std::string, TokenType> keywords;

    CharClass getCharClass(char c) {
        if (std::isalpha(c) || c == '_') return C_LET;
        if (std::isdigit(c)) return C_DIG;
        if (c == '.') return C_DOT;
        if (std::isspace(c)) return C_SPACE;
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' || c == '!') return C_OPCHAR;
        if (c == '(' || c == ')' || c == '[' || c == ']' || c == ';') return C_SEP;
        if (c == '\0') return C_EOF;
        return C_OTHER;
    }

public:
    Lexer(const std::string& src) : source(src), pos(0), line(1), col(1) {
        // Инициализация таблицы переходов
        for (int i = 0; i < 7; ++i)
            for (int j = 0; j < 8; ++j)
                transitionTable[i][j] = -1;

        // S_START (0)
        transitionTable[S_START][C_LET] = S_ID;
        transitionTable[S_START][C_DIG] = S_NUM;
        transitionTable[S_START][C_DOT] = S_ERROR;
        transitionTable[S_START][C_SPACE] = S_START;
        transitionTable[S_START][C_OPCHAR] = S_OP;
        transitionTable[S_START][C_SEP] = S_SEP;
        transitionTable[S_START][C_OTHER] = S_ERROR;

        // S_ID (1)
        transitionTable[S_ID][C_LET] = S_ID;
        transitionTable[S_ID][C_DIG] = S_ID;
        transitionTable[S_ID][C_SPACE] = S_START;
        transitionTable[S_ID][C_OPCHAR] = S_START;
        transitionTable[S_ID][C_SEP] = S_START;
        transitionTable[S_ID][C_DOT] = S_ERROR;

        // S_NUM (2)
        transitionTable[S_NUM][C_DIG] = S_NUM;
        transitionTable[S_NUM][C_DOT] = S_FLOAT;
        transitionTable[S_NUM][C_SPACE] = S_START;
        transitionTable[S_NUM][C_OPCHAR] = S_START;
        transitionTable[S_NUM][C_SEP] = S_START;
        transitionTable[S_NUM][C_LET] = S_ERROR;

        // S_FLOAT (3)
        transitionTable[S_FLOAT][C_DIG] = S_FLOAT;
        transitionTable[S_FLOAT][C_SPACE] = S_START;
        transitionTable[S_FLOAT][C_OPCHAR] = S_START;
        transitionTable[S_FLOAT][C_SEP] = S_START;
        transitionTable[S_FLOAT][C_DOT] = S_ERROR;
        transitionTable[S_FLOAT][C_LET] = S_ERROR;

        // S_OP (4)
        transitionTable[S_OP][C_OPCHAR] = S_OP;
        transitionTable[S_OP][C_SPACE] = S_START;
        transitionTable[S_OP][C_SEP] = S_START;
        transitionTable[S_OP][C_DIG] = S_START;
        transitionTable[S_OP][C_LET] = S_START;
        transitionTable[S_OP][C_DOT] = S_START;

        // S_SEP (5)
        transitionTable[S_SEP][C_SPACE] = S_START;
        transitionTable[S_SEP][C_OPCHAR] = S_START;
        transitionTable[S_SEP][C_SEP] = S_START;
        transitionTable[S_SEP][C_LET] = S_START;
        transitionTable[S_SEP][C_DIG] = S_START;

        // S_ERROR (6)
        for (int j = 0; j < 8; ++j) transitionTable[S_ERROR][j] = S_ERROR;

        // Ключевые слова
        keywords["READ"] = T_KW_READ;
        keywords["WRITE"] = T_KW_WRITE;
        keywords["IF"] = T_KW_IF;
        keywords["THEN"] = T_KW_THEN;
        keywords["ELSE"] = T_KW_ELSE;
        keywords["BEGIN"] = T_KW_BEGIN;
        keywords["END"] = T_KW_END;
        keywords["WHILE"] = T_KW_WHILE;
        keywords["DO"] = T_KW_DO;
        keywords["SQRT"] = T_KW_SQRT;
        keywords["EXP"] = T_KW_EXP;
        keywords["LOG"] = T_KW_LOG;
    }

    Token nextToken() {
        std::string tokenStr = "";
        State currentState = S_START;
        Token result;
        result.type = T_EOF;
        result.value = "";
        result.line = line;
        result.col = col;

        // Пропуск пробелов перед токеном
        while (pos < source.size() && std::isspace(source[pos])) {
            if (source[pos] == '\n') { line++; col = 1; }
            else { col++; }
            pos++;
        }

        if (pos >= source.size()) {
            result.type = T_EOF;
            return result;
        }

        while (true) {
            if (pos >= source.size()) {
                // Конец файла во время токена
                if (currentState != S_START) break;
                result.type = T_EOF;
                return result;
            }

            char c = source[pos];
            CharClass cc = getCharClass(c);

            if (cc == C_EOF) {
                if (currentState != S_START) break;
                result.type = T_EOF;
                return result;
            }

            int nextState = transitionTable[currentState][cc];

            if (nextState == -1 || nextState == S_ERROR) {
                result.type = T_ERROR;
                result.value = std::string(1, c);
                pos++; // Сдвигаем, чтобы не зациклиться
                return result;
            }

            // Завершение токена
            if (currentState != S_START && nextState == S_START) {
                break;
            }

            tokenStr += c;
            currentState = (State)nextState;
            pos++;
            col++;
            if (c == '\n') { line++; col = 1; }
        }

        // Определение типа токена
        if (currentState == S_ID) {
            if (keywords.count(tokenStr)) {
                result.type = keywords[tokenStr];
            }
            else {
                result.type = T_ID;
            }
        }
        else if (currentState == S_NUM) {
            result.type = T_INT;
        }
        else if (currentState == S_FLOAT) {
            result.type = T_FLOAT;
        }
        else if (currentState == S_OP) {
            result.type = resolveOperator(tokenStr);
        }
        else if (currentState == S_SEP) {
            result.type = resolveSeparator(tokenStr);
        }
        else {
            result.type = T_ERROR;
        }

        result.value = tokenStr;
        return result;
    }

private:
    TokenType resolveOperator(const std::string& op) {
        if (op == "=") return T_OP_ASSIGN;
        if (op == "+") return T_OP_ADD;
        if (op == "-") return T_OP_SUB;
        if (op == "*") return T_OP_MUL;
        if (op == "/") return T_OP_DIV;
        if (op == "<") return T_OP_LT;
        if (op == ">") return T_OP_GT;
        if (op == "<=") return T_OP_LE;
        if (op == ">=") return T_OP_GE;
        if (op == "==") return T_OP_EQ;
        if (op == "!=") return T_OP_NE;
        return T_ERROR;
    }

    TokenType resolveSeparator(const std::string& sep) {
        if (sep == "(") return T_SEP_LPAR;
        if (sep == ")") return T_SEP_RPAR;
        if (sep == "[") return T_SEP_LBRACK;
        if (sep == "]") return T_SEP_RBRACK;
        if (sep == ";") return T_SEP_SEMI;
        return T_ERROR;
    }
};