rm -f *.o
lex Parser.l
g++ -g -c lex.yy.c -o lex.yy.o
g++ -g -c create_validate.c -o create_validate.o
g++ -g -c Sql_validate.c -o Sql_validate.o
g++ -g lex.yy.o create_validate.o Sql_validate.o -o pyjamaDB -lfl