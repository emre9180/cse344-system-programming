#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "gtuStudentGrades.h"
#include "../Log/log.h"

void gtuStudentGrades(const char *file)
{
    int fd = open(file, O_WRONLY | O_CREAT | O_EXCL, 0644); // Open the file in write-only mode, create if not exists, and fail if already exists
    if (fd == -1)
    {
        // strcpy(filename, file);
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", file);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    // strcpy(filename, file);
    printf("Filename set to: %s\n", file);

    close(fd); // Close the file
    char logMessage[100];
    sprintf(logMessage, "File created: %s", file);
    logToFile(logMessage);
    exit(EXIT_SUCCESS);
}