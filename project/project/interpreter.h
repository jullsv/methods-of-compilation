#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <stdexcept>
#include "ops.h"

class Interpreter {
private:
    std::vector<double> stack;
    std::map<std::string, double> vars;
    std::map<std::string, std::vector<double>> arrays;

    double pop() {
        if (stack.empty()) throw std::runtime_error("Runtime Error: Stack underflow");
        double val = stack.back();
        stack.pop_back();
        return val;
    }

    void push(double val) {
        stack.push_back(val);
    }

public:
    Interpreter() {}

    void run(const std::vector<Instruction>& program) {
        int pc = 0;
        while (pc < static_cast<int>(program.size())) {
            const Instruction& instr = program[pc];
            try {
                switch (instr.op) {
                case OP_ADD: { double b = pop(); double a = pop(); push(a + b); break; }
                case OP_SUB: { double b = pop(); double a = pop(); push(a - b); break; }
                case OP_MUL: { double b = pop(); double a = pop(); push(a * b); break; }
                case OP_DIV: { double b = pop(); double a = pop(); if (b == 0) throw std::runtime_error("Div by zero"); push(a / b); break; }
                case OP_NEG: { double a = pop(); push(-a); break; }
                case OP_LT: { double b = pop(); double a = pop(); push(a < b ? 1.0 : 0.0); break; }
                case OP_GT: { double b = pop(); double a = pop(); push(a > b ? 1.0 : 0.0); break; }
                case OP_LE: { double b = pop(); double a = pop(); push(a <= b ? 1.0 : 0.0); break; }
                case OP_GE: { double b = pop(); double a = pop(); push(a >= b ? 1.0 : 0.0); break; }
                case OP_EQ: { double b = pop(); double a = pop(); push(a == b ? 1.0 : 0.0); break; }
                case OP_NE: { double b = pop(); double a = pop(); push(a != b ? 1.0 : 0.0); break; }
                case OP_PUSH_INT: { push(std::stod(instr.arg)); break; }
                case OP_PUSH_FLOAT: { push(std::stod(instr.arg)); break; }
                case OP_PUSH_VAR: {
                    if (vars.find(instr.arg) == vars.end()) push(0.0);
                    else push(vars[instr.arg]);
                    break;
                }
                case OP_POP_VAR: { double val = pop(); vars[instr.arg] = val; break; }
                case OP_ARRAY_GET: {
                    double index = pop();
                    int idx = static_cast<int>(index);
                    if (arrays.find(instr.arg) == arrays.end() || idx < 0 || idx >= arrays[instr.arg].size()) {
                        throw std::runtime_error("Array error: " + instr.arg);
                    }
                    push(arrays[instr.arg][idx]);
                    break;
                }
                case OP_ARRAY_SET: {
                    double val = pop();
                    double index = pop();
                    int idx = static_cast<int>(index);
                    if (arrays.find(instr.arg) == arrays.end()) {
                        arrays[instr.arg] = std::vector<double>(idx + 1, 0.0);
                    }
                    else if (idx >= arrays[instr.arg].size()) {
                        arrays[instr.arg].resize(idx + 1, 0.0);
                    }
                    arrays[instr.arg][idx] = val;
                    break;
                }
                case OP_CALL_SQRT: { double a = pop(); if (a < 0) throw std::runtime_error("Sqrt error"); push(std::sqrt(a)); break; }
                case OP_CALL_EXP: { double a = pop(); push(std::exp(a)); break; }
                case OP_CALL_LOG: { double a = pop(); if (a <= 0) throw std::runtime_error("Log error"); push(std::log(a)); break; }
                case OP_JMP: { pc = instr.label; continue; }
                case OP_JMP_FALSE: { double cond = pop(); if (cond == 0.0) { pc = instr.label; continue; } break; }
                case OP_READ: { std::cout << "Enter value: "; double val; std::cin >> val; push(val); break; }
                case OP_PRINT: {
                    double val = pop();
                    if (val == static_cast<int>(val)) std::cout << static_cast<int>(val) << std::endl;
                    else std::cout << val << std::endl;
                    break;
                }
                case OP_END: { return; }
                default: throw std::runtime_error("Unknown instruction");
                }
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << " at instruction " << pc << std::endl;
                return;
            }
            pc++;
        }
    }
};