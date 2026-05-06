# Cемантические действия для генерации ОПС
Ключевые правила расстановки действий:

- Операнды (T_INT, T_FLOAT, T_ID) выводятся в выходной поток сразу при их распознавании.
- Бинарные операторы (+, -, *, /, сравнения) выводятся после рекурсивного разбора правого операнда. Это гарантирует, что к моменту добавления оператора оба его аргумента уже находятся в выходной последовательности.
- Унарный минус заменяется на специальную операцию NEG в ОПЗ.
- Скобки ( ) не выводятся в ОПЗ, так как порядок вычислений полностью определяется структурой дерева разбора.
- Функции (SQRT, EXP, LOG) выводятся после разбора их аргумента, что соответствует постфиксному вызову.
  
## Грамматика с встроенными семантическими действиями
```
1. Выражения

E            → T_INT          { OUT.push(T_INT); } E_Tail
             | T_FLOAT        { OUT.push(T_FLOAT); } E_Tail
             | T_ID           { OUT.push(T_ID); } E_ID_Tail
             | T_SEP_LPAR E T_SEP_RPAR E_Tail
             | T_KW_SQRT T_SEP_LPAR E T_SEP_RPAR { OUT.push("SQRT"); } E_Tail
             | T_KW_EXP  T_SEP_LPAR E T_SEP_RPAR { OUT.push("EXP"); }  E_Tail
             | T_KW_LOG  T_SEP_LPAR E T_SEP_RPAR { OUT.push("LOG"); }  E_Tail

Хвост сложения/вычитания

E_Tail       → T_OP_ADD M    { OUT.push(T_OP_ADD); } E_Tail
             | T_OP_SUB M    { OUT.push(T_OP_SUB); } E_Tail
             | CompTail
             | ε

Хвост сравнений

CompTail     → T_OP_LT M     { OUT.push(T_OP_LT); } CompTail
             | T_OP_GT M     { OUT.push(T_OP_GT); } CompTail
             | T_OP_LE M     { OUT.push(T_OP_LE); } CompTail
             | T_OP_GE M     { OUT.push(T_OP_GE); } CompTail
             | T_OP_EQ M     { OUT.push(T_OP_EQ); } CompTail
             | T_OP_NE M     { OUT.push(T_OP_NE); } CompTail
             | ε

2. Мультипликативные операции

M            → U M_Tail

M_Tail       → T_OP_MUL U    { OUT.push(T_OP_MUL); } M_Tail
             | T_OP_DIV U    { OUT.push(T_OP_DIV); } M_Tail
             | ε

3. Унарные операции и первичные элементы

U            → T_OP_SUB U    { OUT.push("NEG"); }
             | T_INT          { OUT.push(T_INT); }
             | T_FLOAT        { OUT.push(T_FLOAT); }
             | T_ID U_ID_Tail
             | T_SEP_LPAR E T_SEP_RPAR
             | T_KW_SQRT T_SEP_LPAR E T_SEP_RPAR { OUT.push("SQRT"); }
             | T_KW_EXP  T_SEP_LPAR E T_SEP_RPAR { OUT.push("EXP"); }
             | T_KW_LOG  T_SEP_LPAR E T_SEP_RPAR { OUT.push("LOG"); }

Хвосты идентификаторов (работа с массивами)

U_ID_Tail    → T_SEP_LBRACK E T_SEP_RBRACK { OUT.push("ARRAY_GET"); }
             | ε

E_ID_Tail    → T_SEP_LBRACK E T_SEP_RBRACK { OUT.push("ARRAY_GET"); } E_Tail
             | E_Tail
```

- E отвечает за сложение/вычитание и сравнения
- M отвечает за умножение/деление
- U отвечает за унарный минус, функции и первичные элементы
