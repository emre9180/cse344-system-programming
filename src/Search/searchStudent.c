#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "../Log/log.h"

char* convertToLower(const char* str) {
    char* result = (char*)malloc((strlen(str) + 1) * sizeof(char)); // Allocate memory for the result string
    if (result == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(result, str); // Copy the original string to the result string

    for (int i = 0; result[i] != '\0'; i++) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] += 32; // Convert uppercase character to lowercase
        }
    }

    return result;
}

void searchStudent(const char *target, const char *filename)
{
    printf("Searching for student: %s in the file %s\n", target, filename);
    int found = 0;
    int fd = open(filename, O_RDONLY); // Open the file in read-only mode
    if (fd == -1)
    {
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    char *buffer = (char *)malloc(1024 * sizeof(char)); // Allocate memory for buffer
    if (buffer == NULL)
    {
        perror("malloc");
        close(fd);
        char logMessage[100];
        sprintf(logMessage, "Error allocating memory for buffer");
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    off_t total_bytes_read = 0; // Keep track of total bytes read from the file

    while ((bytes_read = read(fd, buffer, 1024)) > 0)
    {
        total_bytes_read += bytes_read;
        char *newline = buffer;
        char *current = buffer; // Pointer to keep track of current position in buffer

        while (newline != NULL)
        {
            newline = strchr(newline, '\n'); // Find the newline character
            if (newline == NULL) break; // Reached end of buffer
            *newline = '\0'; // Replace newline with null terminator
                             
            char *name = strtok(current, ","); // Extract name
            char *grade = strtok(NULL, ",");  // Extract grade

            char *name_lower = convertToLower(name);
            char *target_lower = convertToLower(target);

            if (name != NULL && strcmp(name_lower, target_lower) == 0)
            {
                found = 1;
                printf("The record has been found: Name: %s, Grade: %s\n", name, grade);
                free(name_lower);
                free(target_lower);
                break;
            }
            newline++; // Move past the null terminator
            current = newline; // Update current pointer to point to the next line
        }
        lseek(fd, total_bytes_read, SEEK_SET);
        // Clear the buffer
        memset(buffer, 0, 1024); 
    }

    if (bytes_read == -1)
    {
        perror("read");
        close(fd);
        free(buffer); // Free the allocated memory
        char logMessage[100];
        sprintf(logMessage, "Error reading file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }
    if(found==0){
        printf("The record has not been found\n");
    }
    
    close(fd);    // Close the file
    free(buffer); // Free the allocated memory
    char logMessage[100];
    sprintf(logMessage, "Successful command: %s", "search");
    logToFile(logMessage);
    exit(EXIT_SUCCESS);
}
