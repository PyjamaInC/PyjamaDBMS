rm -f *.o

# Parser directory
lex Parser.l
g++ -g -c lex.yy.c -o lex.yy.o
g++ -g -c create_validate.c -o create_validate.o
g++ -g -c Sql_validate.c -o Sql_validate.o
g++ -g -c -fpermissive ../B_tree/B_tree.c -o  ../B_tree/B_tree.o

# core directory
g++ -g -c ../core/create_statement.c -o ../core/create_statement.o

g++ -g lex.yy.o create_validate.o Sql_validate.o ../core/create_statement.o ../B_tree/B_tree.o -o pyjamaDB -lfl
