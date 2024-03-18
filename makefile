CC = gcc-13
CFLAGS = -Wall -Wextra -g

all: main

main: main.o sort.o display.o addStudentGrade.o gtuStudentGrades.o searchStudent.o
	$(CC) $(CFLAGS) -o main main.o searchStudent.o sort.o display.o addStudentGrade.o gtuStudentGrades.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

searchStudent.o: src/Search/searchStudent.c
	$(CC) $(CFLAGS) -c src/Search/searchStudent.c

sort.o: src/Sort/sort.c
	$(CC) $(CFLAGS) -c src/Sort/sort.c

display.o: src/Display/display.c
	$(CC) $(CFLAGS) -c src/Display/display.c

addStudentGrade.o: src/Add/addStudentGrade.c
	$(CC) $(CFLAGS) -c src/Add/addStudentGrade.c

gtuStudentGrades.o: src/FileCreation/gtuStudentGrades.c
	$(CC) $(CFLAGS) -c src/FileCreation/gtuStudentGrades.c

clean:
	rm -f main main.o searchStudent.o sort.o display.o addStudentGrade.o gtuStudentGrades.o    
