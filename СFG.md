# 2. КС-грамматика языка (Исходная)

$G = (N, \Sigma, P, S)$

где:

- \($\Sigma\$) — множество терминалов (токенов из п. 1.1)
- \(N\) — множество нетерминалов
- \(P\) — множество правил вывода
- \(S\) — начальный символ грамматики

```
Program      → StatementList
StatementList → Statement StatementList | ε

Statement    → Assignment 
             | IfStmt 
             | WhileStmt 
             | ReadStmt 
             | WriteStmt

Assignment   → T_ID T_OP_ASSIGN Expression T_SEP_SEMI

IfStmt       → T_KW_IF Expression T_KW_THEN StatementList ElsePart T_KW_END T_SEP_SEMI
ElsePart     → T_KW_ELSE StatementList | ε

WhileStmt    → T_KW_WHILE Expression T_KW_DO StatementList T_KW_END T_SEP_SEMI

ReadStmt     → T_KW_READ T_ID T_SEP_SEMI
WriteStmt    → T_KW_WRITE Expression T_SEP_SEMI

Expression   → Comparison

Comparison   → Addition CompOp Addition | Addition
CompOp       → T_OP_LT | T_OP_GT | T_OP_LE | T_OP_GE | T_OP_EQ | T_OP_NE

Addition     → Multiplication T_OP_ADD Addition 
             | Multiplication T_OP_SUB Addition 
             | Multiplication

Multiplication → Unary T_OP_MUL Multiplication 
               | Unary T_OP_DIV Multiplication 
               | Unary

Unary        → T_OP_SUB Unary | Primary 

Primary      → T_INT 
             | T_FLOAT 
             | T_ID ArrayIndex
             | T_SEP_LPAR Expression T_SEP_RPAR

ArrayIndex   → T_SEP_LBRACK Expression T_SEP_RBRACK
```

# 3. КС-грамматика в нестрогой нормальной форме Грейбах (НФГ)

Для реализации нисходящего синтаксического анализатора (магазинного автомата) грамматика преобразована так, что каждое правило начинается с терминала. Левая рекурсия устранена путем введения дополнительных нетерминалов ("хвостов").
```
// 1. Список операторов (Главный нетерминал S)
// Каждое правило начинается с терминала (ключевого слова или T_ID)
S            → T_ID T_OP_ASSIGN E T_SEP_SEMI S          // Присваивание
             | T_KW_IF E T_KW_THEN S ElsePart T_KW_END T_SEP_SEMI S // Условие
             | T_KW_WHILE E T_KW_DO S T_KW_END T_SEP_SEMI S         // Цикл
             | T_KW_READ T_ID T_SEP_SEMI S                          // Ввод
             | T_KW_WRITE E T_SEP_SEMI S                            // Вывод
             | ε                                                    // Конец программы

ElsePart     → T_KW_ELSE S | ε

// 2. Выражения (E)
// Начинаются с терминала. Хвост E_Tail обрабатывает + и -
E            → T_INT E_Tail
             | T_FLOAT E_Tail
             | T_ID E_ID_Tail       // Переменная или массив
             | T_SEP_LPAR E T_SEP_RPAR E_Tail  // Скобки
             | T_KW_SQRT T_SEP_LPAR E T_SEP_RPAR E_Tail
             | T_KW_EXP T_SEP_LPAR E T_SEP_RPAR E_Tail
             | T_KW_LOG T_SEP_LPAR E T_SEP_RPAR E_Tail

// Хвост выражения: Сложение/Вычитание (низший приоритет в формулах)
E_Tail       → T_OP_ADD M E_Tail
             | T_OP_SUB M E_Tail
             | CompTail           // Переход к сравнению, если + - закончились
             | ε

// Хвост сравнения: Операции < > == != (используются в IF/WHERE)
CompTail     → T_OP_LT M CompTail
             | T_OP_GT M CompTail
             | T_OP_LE M CompTail
             | T_OP_GE M CompTail
             | T_OP_EQ M CompTail
             | T_OP_NE M CompTail
             | ε

// 3. Мультипликативные операции (M)
// Начинаются с первичного элемента U. Хвост M_Tail обрабатывает * и /
M            → U M_Tail

M_Tail       → T_OP_MUL U M_Tail
             | T_OP_DIV U M_Tail
             | ε

// 4. Первичные элементы и унарные операции (U)
U            → T_OP_SUB U             // Унарный минус
             | T_INT                  // Целое число
             | T_FLOAT                // Вещественное число
             | T_ID U_ID_Tail         // Идентификатор (возможно с индексом)
             | T_SEP_LPAR E T_SEP_RPAR // Выражение в скобках
             | T_KW_SQRT T_SEP_LPAR E T_SEP_RPAR
             | T_KW_EXP T_SEP_LPAR E T_SEP_RPAR
             | T_KW_LOG T_SEP_LPAR E T_SEP_RPAR

// Хвост идентификатора в первичном элементе (только массивы)
U_ID_Tail    → T_SEP_LBRACK E T_SEP_RBRACK
             | ε

// 5. Хвост идентификатора в выражении (E_ID_Tail)
// Отличается от U_ID_Tail тем, что после него может идти продолжение выражения (E_Tail)
E_ID_Tail    → T_SEP_LBRACK E T_SEP_RBRACK E_Tail  // arr[i] + ...
             | E_Tail                               // просто x + ...
```
