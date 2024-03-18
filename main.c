#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>

#include "src/FileCreation/gtuStudentGrades.h"
#include "src/Add/addStudentGrade.h"
#include "src/Display/display.h"
#include "src/Search/searchStudent.h"
#include "src/Sort/sort.h"

#define MAX_INPUT_LENGTH 100

int main()
{
    char input[MAX_INPUT_LENGTH];
    while (1)
    {
        ssize_t bytes_read = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);
        if (bytes_read == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else if (bytes_read == 0)
        {
            break; // End of input
        }

        input[bytes_read] = '\0'; // Null terminate the string

        // Parse input
        char *command = strtok(input, " \n"); // Extract first word as command
        char *arg1 = strtok(NULL, " \n");     // Extract first argument
        char *arg2 = strtok(NULL, " \n");     // Extract second argument
        char *arg3 = strtok(NULL, " \n");     // Extract third argument
        char *file = strtok(NULL, "\"\n");    // Extract fourth argument

        printf("%s, %s, %s, %s, %s\n", command, arg1, arg2, arg3, file);

        if (command != NULL)
        {
            if (strcmp(command, "exit") == 0 && arg1 == NULL)
            {
                exit(EXIT_SUCCESS);
            }
            if (strcmp(command, "gtuStudentGrades") == 0 && arg1 != NULL)
            {
                gtuStudentGrades(arg1);
                continue;
            }

            pid_t pid = fork(); // Fork the process
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                if (strcmp(command, "addStudentGrade") == 0 && arg1 != NULL && arg2 != NULL, arg3 != NULL)
                {
                    addStudentGrade(file, arg1, arg2, arg3);
                }
                else if (strcmp(command, "searchStudent") == 0 && arg1 != NULL)
                {
                    searchStudent(file, arg1);
                }
                else if (strcmp(command, "sortAll") == 0)
                {
                    sortAll(arg1);
                }
                else if (strcmp(command, "showAll") == 0)
                {
                    showAll(arg1, -1, -1);
                }
                else if (strcmp(command, "lstGrades") == 0)
                {
                    showAll(arg1, 5, -1);
                }
                else if (strcmp(command, "lstSome") == 0 && arg1 != NULL && arg2 != NULL && arg3 != NULL)
                {
                    int numEntries;
                    int pageNumber;

                    if (arg1 == NULL)
                        numEntries = -1;
                    else
                        numEntries = atoi(arg1);

                    if (arg2 == NULL)
                        pageNumber = -1;
                    else
                        pageNumber = atoi(arg2);

                    showAll(arg3, numEntries, pageNumber);
                }
                else if (strcmp(command, "quit") == 0)
                {
                    exit(EXIT_SUCCESS); // Exit child process
                }
                else
                {
                    printf("Invalid command or arguments.\n");
                    exit(EXIT_SUCCESS); // Exit child process (error code
                }
                exit(EXIT_SUCCESS); // Exit child process
            }
            else
            {
                // Parent process
                int status;
                waitpid(pid, &status, 0); // Wait for child process to finish
            }
        }
    }

    exit(EXIT_SUCCESS); // Exit child process
}
