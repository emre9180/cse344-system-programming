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
    char *buffer = (char *)malloc(1024 * sizeof(char));
    if (buffer == NULL)
    {
        perror("malloc");
        char logMessage[100];
        sprintf(logMessage, "Error allocating memory for buffer");
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_read;
    int ct = 0;
    off_t total_bytes_read = 0; // Keep track of total bytes read from the file

    printf("Displaying all student grades\n");
    while ((bytes_read = read(fd, buffer, 1024)) > 0)
    {
        total_bytes_read += bytes_read;
        char *newline = buffer;
        char *current = buffer; // Pointer to keep track of current position in buffer

        while (newline != NULL)
        {
            newline = strchr(newline, '\n'); // Find the newline character
            if (newline == NULL)
                break;       // Reached end of buffer
            *newline = '\0'; // Replace newline with null terminator

            // Process the line
            int printFlag = 1;
            if (ct == 5 && numEntries == 5 && pageNumber == -1)
            {
                // Handle special condition
            }
            else if (numEntries != -1 && pageNumber != -1 && ((ct > (pageNumber * numEntries) - 1) || ct < ((pageNumber - 1) * numEntries)))
            {
                printFlag = 0;
            }
            if (printFlag == 1)
            {
                printf("Record %d: %s\n", ct, current); // Print current line
            }

            newline++;         // Move past the null terminator
            current = newline; // Update current pointer to point to the next line
            ct++;
        }

        // Move the file pointer to the next line
        lseek(fd, total_bytes_read, SEEK_SET);

        // Clear the buffer
        memset(buffer, 0, 1024);
    }

    if (bytes_read == -1)
    {
        perror("read");
        // Log error and exit
    }

    close(fd);
    free(buffer);
    // Exit normally

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
