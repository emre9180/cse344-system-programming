#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "../Log/log.h"

void showAll(const char *filename, int numEntries, int pageNumber)
{
    int fd = open(filename, O_RDWR); // Open the file in read-write mode
    if (fd == -1)
    {
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }
    // Read the file and store the lines in a string array
    char buffer[1024];
    ssize_t bytes_read;
    char *newline;
    int ct = 0;

    printf("Displaying all student grades\n");
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        newline = strchr(buffer, '\n'); // Find the newline character

        while (newline != NULL)
        {
            if (*(newline + 1) == '\0')
            {
                break;
            }
            int printFlag = 1;
            if (ct == 5 && numEntries == 5 && pageNumber == -1)
            {
                char logMessage[100];
                sprintf(logMessage, "Successful command: %s", "Print first 5 students");
                logToFile(logMessage);
                close(fd);
                exit(EXIT_SUCCESS);
            }

            else if (numEntries != -1 && pageNumber != -1 && ((ct > (pageNumber * numEntries) - 1) || ct < ((pageNumber - 1) * numEntries)))
            {
                printFlag = 0;
            }
            *newline = '\0'; // Replace newline with null terminator
            if (printFlag == 1)
                printf("Record %d: %s\n", ct, buffer);

            // buffer[newline - buffer] = '\n';    // Restore newline character
            strcpy(buffer, newline + 1);        // Update buffer to start from the new newline pointer
            newline = strchr(buffer + 1, '\n'); // Find the next newline character
            ct++;

            // for taking last string of the document. could be deleted
            // if ((newline == NULL || *newline == '\0') && printFlag == 1)
            // {
            //     printf("Record %d: %s\n", ct, buffer);
            //     break;
            // }
            // Print name, surname and grade
        }

        lseek(fd, newline - buffer + 1, SEEK_CUR); // Move the file pointer to the next line
        strcpy(buffer, "");                        // Clear the buffer
    }
    if (bytes_read == -1)
    {
        perror("read");
        char logMessage[100];
        sprintf(logMessage, "Error reading file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    if (pageNumber == -1)
    {
        char logMessage[100];
        sprintf(logMessage, "Successful command: %s", "Print all students");
        logToFile(logMessage);
    }
    else
    {
        char logMessage[100];
        sprintf(logMessage, "Successful command: %s", "Print a page of students");
        logToFile(logMessage);
    }
    close(fd); // Close the file
    exit(EXIT_SUCCESS);
}
