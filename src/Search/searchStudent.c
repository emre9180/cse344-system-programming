
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>

void searchStudent(const char *target, const char *filename)
{
    printf("Searching for student: %s\n", target);

    int fd = open(filename, O_RDONLY); // Open the file in read-only mode
    if (fd == -1)
    {
        perror("open");
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;
    char *newline;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        newline = strchr(buffer, '\n'); // Find the newline character
        // printf("%s, buffer", buffer);

        while (newline != NULL)
        {
            *newline = '\0';                   // Replace newline with null terminator
            char *name = strtok(buffer, " ");  // Extract name
            char *surname = strtok(NULL, " "); // Extract surname
            char *grade = strtok(NULL, " ");   // Extract grade
            if (name != NULL && strcmp(name, target) == 0)
            {
                printf("The record has been found: Name: %s, Surname: %s, Grade: %s\n", name, surname, grade);
                break;
            }
            // buffer[newline - buffer] = '\n';    // Restore newline character
            strcpy(buffer, newline + 1);        // Update buffer to start from the new newline pointer
            newline = strchr(buffer + 1, '\n'); // Find the next newline character

            // for taking last string of the document. could be deleted
            if (newline == NULL || *newline == '\0')
            {
                char *name = strtok(buffer, " ");  // Extract name
                char *surname = strtok(NULL, " "); // Extract surname
                char *grade = strtok(NULL, " ");   // Extract grade
                if (name != NULL && strcmp(name, target) == 0)
                {
                    printf("The record has been found: Name: %s, Surname: %s, Grade: %s\n", name, surname, grade);
                    break;
                }
            }
        }

        lseek(fd, newline - buffer + 1, SEEK_CUR); // Move the file pointer to the next line
        strcpy(buffer, "");                        // Clear the buffer
    }

    if (bytes_read == -1)
    {
        perror("read");
    }

    close(fd); // Close the file
}
