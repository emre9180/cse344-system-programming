#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "../Log/log.h"
#include "sort.h"

// Function to sort all student grades
LetterGrade findGrade(const char *grade)
{
    // Compare the grade and return the corresponding enum value
    if (strcmp(grade, "AA") == 0)
        return GRADE_AA;
    else if (strcmp(grade, "BA") == 0)
        return GRADE_BA;
    else if (strcmp(grade, "BB") == 0)
        return GRADE_BB;
    else if (strcmp(grade, "CB") == 0)
        return GRADE_CB;
    else if (strcmp(grade, "CC") == 0)
        return GRADE_CC;
    else if (strcmp(grade, "DC") == 0)
        return GRADE_DC;
    else if (strcmp(grade, "DD") == 0)
        return GRADE_DD;
    else if (strcmp(grade, "FF") == 0)
        return GRADE_FF;
    else if (strcmp(grade, "VF") == 0)
        return GRADE_VF;
    else
        return GRADE_NA;
}

// Function to sort all student grades
void sortAll(const char *filename)
{
    int fd = open(filename, O_RDONLY); // Open the file in read-write mode
    if (fd == -1)
    {
        perror("open");
        char logMessage[100];
        sprintf(logMessage, "Error opening file: %s", filename);
        logToFile(logMessage);
        exit(EXIT_FAILURE);
    }

    // Get the sorting option and order from the user
    int sortOption;
    int sortOrder;

    char optionBuffer[2];
    char orderBuffer[2];

    write(STDOUT_FILENO, "Enter the sorting option:\n", 26);
    write(STDOUT_FILENO, "1. Sort by name\n", 17);
    write(STDOUT_FILENO, "2. Sort by grade\n", 18);
    read(STDIN_FILENO, optionBuffer, sizeof(optionBuffer));
    sortOption = atoi(optionBuffer);

    write(STDOUT_FILENO, "Enter the sorting order:\n", 26);
    write(STDOUT_FILENO, "1. Ascending\n", 14);
    write(STDOUT_FILENO, "2. Descending\n", 15);
    read(STDIN_FILENO, orderBuffer, sizeof(orderBuffer));
    sortOrder = atoi(orderBuffer);

    // Read the file and store the lines in a string array
    char *buffer = (char *)malloc(1024 * sizeof(char));
    ssize_t bytes_read;
    char **lines = (char **)malloc(1024 * sizeof(char *)); // Assuming a maximum of 100 lines
    int numLines = 0;
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

            lines[numLines] = strdup(current); // Allocate memory and store the line in the array
            numLines++;

            newline++; // Move past the null terminator
            current = newline; // Update current pointer to point to the next line
        }

        lseek(fd, total_bytes_read, SEEK_SET);
        memset(buffer, 0, 1024);                   // Clear the buffer
    }

    if (bytes_read == -1)
    {
        char logMessage[100];
        sprintf(logMessage, "Error reading file: %s", filename);
        logToFile(logMessage);
        close(fd); // Close the file
        free(buffer);
        for (int i = 0; i < numLines; i++)
        {
            free(lines[i]);
        }
        free(lines);
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd); // Close the file

    // Sort the lines in ascending order based on names
    if (sortOption == 1)
    {
        for (int i = 0; i < numLines - 1; i++)
        {
            for (int j = 0; j < numLines - i - 1; j++)
            {
                char *name1 = strdup(lines[j]);
                char *name2 = strdup(lines[j + 1]);

                if (strcmp(name1, name2) > 0)
                {
                    char *temp = lines[j];
                    lines[j] = lines[j + 1];
                    lines[j + 1] = temp;
                }
            }
        }
    }

    // Sort the lines in ascending order based on grades
    else
    {
        for (int i = 0; i < numLines - 1; i++)
        {
            for (int j = 0; j < numLines - i - 1; j++)
            {
                char *last_space1 = strrchr(lines[j], ' ');
                char *last_space2 = strrchr(lines[j + 1], ' ');

                if (last_space1 && last_space2)
                {
                    // Calculate the length of the grade string
                    size_t len1 = strlen(last_space1 + 1);
                    size_t len2 = strlen(last_space2 + 1);

                    // Allocate memory for grade strings
                    char *grade1 = malloc(len1 + 1);
                    char *grade2 = malloc(len2 + 1);

                    // Copy grade strings and null-terminate
                    strncpy(grade1, last_space1 + 1, len1);
                    grade1[len1] = '\0';

                    strncpy(grade2, last_space2 + 1, len2);
                    grade2[len2] = '\0';

                    LetterGrade letterGrade1 = findGrade(grade1);
                    LetterGrade letterGrade2 = findGrade(grade2);

                    // Free allocated memory
                    free(grade1);
                    free(grade2);

                    if (letterGrade1 < letterGrade2)
                    {
                        char *temp = lines[j];
                        lines[j] = lines[j + 1];
                        lines[j + 1] = temp;
                    }
                }
            }
        }
    }
    printf("Displaying all student grades\n");

    // Print the sorted lines based on the sorting order
    if (sortOrder == 2)
    {
        for (int i = numLines - 1; i >= 0; i--)
        {
            printf("Record %d: %s\n", numLines - i, lines[i]);
        }
    }

    // Print the sorted lines based on the sorting order
    else
    {
        for (int i = 0; i < numLines; i++)
        {
            printf("Record %d: %s\n", i + 1, lines[i]);
        }
    }

    char logMessage[100];
    sprintf(logMessage, "Successful command: %s", "Sort all students");
    logToFile(logMessage);

    // Free dynamically allocated memory
    free(buffer);
    for (int i = 0; i < numLines; i++)
    {
        free(lines[i]);
    }
    free(lines);

    exit(EXIT_SUCCESS);
}
