#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "src/FileCreation/gtuStudentGrades.h"
#include "src/Add/addStudentGrade.h"
#include "src/Display/display.h"
#include "src/Search/searchStudent.h"
#include "src/Sort/sort.h"
#include "src/Log/log.h"

#define MAX_INPUT_LENGTH 100

int main()
{
    printf("Welcome to GTU Student Management System\n");
    char input[MAX_INPUT_LENGTH];
    while (1)
    {
        int multiple_name = 0;
        ssize_t bytes_read = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);
        if (bytes_read == -1)
        {
            perror("read");
            logToFile("Error reading command");
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

        if (arg1 && arg1[0] == '"' && (strcmp(command, "addStudentGrade") == 0 || strcmp(command, "searchStudent") == 0))
        {
            char *read;
            // If first character is a quote, extract until the next quote
            read = strtok(NULL, "\"\n");
            // Concatenate first_char and arg1
            char concatenated_arg1[MAX_INPUT_LENGTH];
            strcpy(concatenated_arg1, arg1);
            strcat(concatenated_arg1, " ");
            strcat(concatenated_arg1, read);
            strcpy(arg1, concatenated_arg1);
            multiple_name = 1;
        }

        char *arg2 = strtok(NULL, " \n"); // Extract second argument
        char *arg3 = strtok(NULL, " \n"); // Extract third argument
        char *file = strtok(NULL, " \n");

        if (arg1 != NULL)
        {
            if (arg1[0] == '"')
                arg1++; // Move the pointer to the character after the first quote
            if (arg1[strlen(arg1) - 1] == '"')
                arg1[strlen(arg1) - 1] = '\0'; // Remove the last quote
        }

        if (arg2 != NULL)
        {
            if (arg2[0] == '"')
                arg2++; // Move the pointer to the character after the first quote
            if (arg2[strlen(arg2) - 1] == '"')
                arg2[strlen(arg2) - 1] = '\0'; // Remove the last quote
        }

        if (arg3 != NULL)
        {
            if (arg3[0] == '"')
                arg3++; // Move the pointer to the character after the first quote
            if (arg3[strlen(arg3) - 1] == '"')
                arg3[strlen(arg3) - 1] = '\0'; // Remove the last quote
        }

        if (file != NULL)
        {
            if (file[0] == '"')
                file++; // Move the pointer to the character after the first quote
            if (file[strlen(file) - 1] == '"')
                file[strlen(file) - 1] = '\0'; // Remove the last quote
        }

        if (command != NULL)
        {
            if (strcmp(command, "exit") == 0 && arg1 == NULL)
            {
                if (command != NULL)
                {
                    if (strcmp(command, "exit") == 0 && arg1 == NULL)
                    {
                        char logMessage[100];
                        sprintf(logMessage, "Successful command: %s", command);
                        logToFile(logMessage);
                        exit(EXIT_SUCCESS);
                    }
                    // Rest of the code...
                }
            }

            pid_t pid = fork(); // Fork the process
            if (pid == -1)
            {
                perror("fork");
                logToFile("Error forking process");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                if (strcmp(command, "gtuStudentGrades") == 0 && arg1 != NULL)
                {
                    gtuStudentGrades(arg1);
                    continue;
                }

                if (strcmp(command, "gtuStudentGrades") == 0 && arg1 == NULL)
                {
                    usage();
                    exit(EXIT_SUCCESS);
                }

                if (strcmp(command, "addStudentGrade") == 0 && arg1 != NULL && arg2 != NULL && arg3 != NULL)
                {
                    if (multiple_name == 0)
                    {
                        char result[MAX_INPUT_LENGTH]; // Adjust the size according to your requirements
                        strcpy(result, arg1);
                        strcat(result, " ");
                        strcat(result, arg2);
                        strcpy(arg1, result);
                        addStudentGrade(file, arg1, arg3);
                    }

                    else
                    {
                        addStudentGrade(arg3, arg1, arg2);
                    }
                }

                else if (strcmp(command, "searchStudent") == 0 && arg1 != NULL && arg2 != NULL)
                {
                    if (multiple_name == 0)
                    {
                        char result[MAX_INPUT_LENGTH]; // Adjust the size according to your requirements
                        strcpy(result, arg1);
                        strcat(result, " ");
                        strcat(result, arg2);
                        strcpy(arg1, result);
                        searchStudent(arg1, arg3);
                    }

                    else
                    {
                        searchStudent(arg1, arg2);
                    }
                }

                else if (strcmp(command, "sortAll") == 0)
                    sortAll(arg1);

                else if (strcmp(command, "showAll") == 0)
                    showAll(arg1, -1, -1);

                else if (strcmp(command, "listGrades") == 0)
                    showAll(arg1, 5, -1);

                else if (strcmp(command, "listSome") == 0 && arg1 != NULL && arg2 != NULL && arg3 != NULL)
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
                    char logMessage[100];
                    sprintf(logMessage, "Successful command: %s", "quit");
                    logToFile(logMessage);
                    exit(EXIT_SUCCESS); // Exit child process
                }
                else
                {
                    printf("Invalid command or arguments.\n");
                    logToFile("Error: Invalid command or arguments");
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
