#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "gtuStudentGrades.h"
#include "addStudentGrade.h"



int main(int argc, char *argv[]) {
    if (argc == 3 || strcmp(argv[1], "gtuStudentGrades") == 0) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            openFile(argv[2]);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            waitpid(pid, NULL, 0);
            printf("File created successfully.\n");
        }
        return 0;
    }




    else if (argc == 4 || strcmp(argv[1], "addStudentGrade") == 0) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            addStudent(argv[2], argv[3]);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            waitpid(pid, NULL, 0);
            printf("File created successfully.\n");
        }
        return 0;
    }

    else {
        printf("Invalid arguments\n");
        return 1;
    }
    
}