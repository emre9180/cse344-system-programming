CC = gcc
CFLAGS = -Wall -Wextra -g

all: main

main: main.o child1.o child2.o
	$(CC) $(CFLAGS) -o main main.o child2.o child1.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

child2.o: child2.c
	$(CC) $(CFLAGS) -c child2.c



child1.o: child1.c
	$(CC) $(CFLAGS) -c child1.c

clean:
	rm -f main main.o child2.o child1.o    
