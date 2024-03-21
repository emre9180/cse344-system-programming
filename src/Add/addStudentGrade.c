#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "../Log/log.h"
#define MAX_INPUT_LENGTH 100

void addStudentGrade(const char *filename, const char *name, char *grade)
{
    printf("Adding student grade: %s, %s to file %s\n", name, grade, filename);
    for (int i = 0; i < strlen(grade); i++)
    {
        if (islower(grade[i]))
        {
            grade[i] = toupper(grade[i]);
        }
    }
    int fd = open(filename, O_WRONLY | O_APPEND); // Open the file in write-only mode and append mode
    if (fd == -1)
    {
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    char *buffer = (char *)malloc(MAX_INPUT_LENGTH * sizeof(char)); // Allocate memory for the buffer
    if (buffer == NULL)
    {
        perror("malloc");
        char logMessage[100];
        sprintf(logMessage, "Error allocating memory for buffer");
        logToFile(logMessage);
        close(fd); // Close the file
        exit(EXIT_FAILURE);
    }

    snprintf(buffer, MAX_INPUT_LENGTH, "%s, %s\n", name, grade); // Format the name and grade as a string
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));   // Write the string to the file
    if (bytes_written == -1)
    {
        perror("write");
        char logMessage[100];
        sprintf(logMessage, "Error reading file: %s", filename);
        logToFile(logMessage);
        close(fd);    // Close the file
        free(buffer); // Free the allocated memory
        exit(EXIT_FAILURE);
    }

    close(fd);    // Close the file
    free(buffer); // Free the allocated memory
    exit(EXIT_SUCCESS);
}