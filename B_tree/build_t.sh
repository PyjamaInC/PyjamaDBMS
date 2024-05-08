g++ -g -c -fpermissive B_tree.c -o B_tree.o
g++ -g -c main.c -o main.o
g++ -g B_tree.o main.o -o test