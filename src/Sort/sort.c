
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>

void sortAll(const char *filename)
{
    int fd = open(filename, O_RDWR); // Open the file in read-write mode
    if (fd == -1)
    {
        perror("open");
        return;
    }

    // Read the file and store the lines in a string array
    char buffer[1024];
    ssize_t bytes_read;
    char *lines[100]; // Assuming a maximum of 100 lines
    char *newline;
    int numLines = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        newline = strchr(buffer, '\n'); // Find the newline character
        // printf("%s, buffer", buffer);

        while (newline != NULL)
        {
            *newline = '\0'; // Replace newline with null terminator

            lines[numLines] = strdup(buffer); // Allocate memory and store the line in the array
            numLines++;

            // buffer[newline - buffer] = '\n';    // Restore newline character
            strcpy(buffer, newline + 1);        // Update buffer to start from the new newline pointer
            newline = strchr(buffer + 1, '\n'); // Find the next newline character

            // for taking last string of the document. could be deleted
            // if (newline == NULL || *newline == '\0')
            // {
            //     lines[numLines] = strdup(buffer);
            //     numLines++;
            //     break;
            // }
        }

        lseek(fd, newline - buffer + 1, SEEK_CUR); // Move the file pointer to the next line
        strcpy(buffer, "");                        // Clear the buffer
    }

    if (bytes_read == -1)
    {
        perror("read");
    }

    close(fd); // Close the file

    // Sort the lines in ascending order based on names
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

    printf("Displaying all student grades\n");

    for (int i = 0; i < numLines; i++)
    {
        printf("Record %d: %s\n", i, lines[i]);
    }

    // // Write the sorted lines back to the file
    // fd = open(filename, O_WRONLY | O_TRUNC); // Open the file in write-only mode and truncate it
    // if (fd == -1)
    // {
    //     perror("open");
    //     return;
    // }

    // for (int i = 0; i < numLines; i++)
    // {
    //     write(fd, lines[i], strlen(lines[i]));
    //     write(fd, "\n", 1);
    //     free(lines[i]); // Free the memory allocated for each line
    // }

    // close(fd); // Close the file
}
