#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "../Log/log.h"
#define MAX_INPUT_LENGTH 100

void addStudentGrade(const char *filename, const char *name, const char *surname, const char *grade)
{
    printf("Adding student grade: %s, %s, %s\n", name, surname, grade);

    int fd = open(filename, O_WRONLY | O_APPEND); // Open the file in write-only mode and append mode
    if (fd == -1)
    {
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_INPUT_LENGTH];
    snprintf(buffer, sizeof(buffer), "%s %s %s\n", name, surname, grade); // Format the name and grade as a string
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));            // Write the string to the file
    if (bytes_written == -1)
    {
        perror("write");
        char logMessage[100];
        sprintf(logMessage, "Error reading file: %s", filename);
        logToFile(logMessage);
        close(fd); // Close the file
        exit(EXIT_FAILURE);
    }

    close(fd); // Close the file
    exit(EXIT_SUCCESS);
}