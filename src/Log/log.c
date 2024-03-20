#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> // Include the fcntl.h header for open system call
#include "string.h"

void logToFile(const char *logString)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Failed to fork the process.\n");
        return;
    }
    else if (pid > 0)
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Child process
        char filePath[] = "../cse344-system-programming/log.txt";
        int file = open(filePath, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (file == -1)
        {
            printf("Failed to open the log file.\n");
            exit(EXIT_FAILURE);
        }

        time_t currentTime = time(NULL);
        struct tm *timeInfo = localtime(&currentTime);
        char timeString[20];
        strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

        char logLine[100];
        snprintf(logLine, sizeof(logLine), "[%s]: %s\n", timeString, logString);
        ssize_t bytesWritten = write(file, logLine, strlen(logLine));
        if (bytesWritten == -1)
        {
            printf("Failed to write to the log file.\n");
            close(file);
            exit(EXIT_FAILURE);
        }

        close(file);
        exit(EXIT_SUCCESS);
    }
}
