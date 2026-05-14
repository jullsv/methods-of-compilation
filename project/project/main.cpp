#include <iostream>
#include <string>
#include <vector>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

// Объявление функции перед использованием
void runTest(const std::string& source);

int main() {
    setlocale(LC_ALL, "Russian");

    // Тест 1: Простая формула с функцией
    std::string code1 = R"(
        BEGIN
        x = 16.0;
        y = SQRT(x) + 2 * 3;
        WRITE y;
        END;
    )";

    // Тест 2: Массив и цикл
    std::string code2 = R"(
        BEGIN
        READ n;
        i = 0;
        WHILE i < n DO
            READ arr[i];
            i = i + 1;
        END;
        WRITE arr[0];
        END;
    )";

    std::cout << "=== Test 1 (Formula) ===" << std::endl;
    runTest(code1);

    std::cout << "\n=== Test 2 (Array) ===" << std::endl;
    runTest(code2); 
    
    return 0;
}

void runTest(const std::string& source) {
    try {
        Lexer lexer(source);
        Parser parser(lexer);
        std::vector<Instruction> ops = parser.parse();

        for (const auto& op : ops) {
            std::cout << "Op: " << op.op << " Arg: " << op.arg << " Label: " << op.label << std::endl;
        }

        Interpreter interpreter;
        interpreter.run(ops);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}