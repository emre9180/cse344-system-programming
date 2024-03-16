CC = gcc
CFLAGS = -Wall -Wextra -g

all: main

main: main.o gtuStudentGrades.o addStudentGrade.o
	$(CC) $(CFLAGS) -o main main.o gtuStudentGrades.o addStudentGrade.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

gtuStudentGrades.o: gtuStudentGrades.c
	$(CC) $(CFLAGS) -c gtuStudentGrades.c

addStudentGrade.o: addStudentGrade.c
	$(CC) $(CFLAGS) -c addStudentGrade.c

clean:
	rm -f main main.o gtuStudentGrades.o addStudentGrade.o
