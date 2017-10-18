yacc -d calc.yacc
lex calc.lex
gcc y.tab.c lex.yy.c my_calc.c -o output
./output