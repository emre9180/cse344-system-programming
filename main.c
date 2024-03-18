#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>

#define MAX_INPUT_LENGTH 100

char filename[MAX_INPUT_LENGTH] = "";

// Function declarations
void addStudentGrade(const char *filename, const char *name, const char *surname, const char *grade);
void searchStudent(const char *filename, const char *name);
void sortAll(const char *filename);
void showAll(const char *filename, int numEntries, int pageNumber);
void lstGrades(const char *filename);
void lstSome(const char *filename, int numEntries, int pageNumber);
void gtuStudentGrades(const char *file);

int main()
{
    char input[MAX_INPUT_LENGTH];
    strcpy(filename, "emre.txt");
    while (1)
    {
        ssize_t bytes_read = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);
        if (bytes_read == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else if (bytes_read == 0)
        {
            break; // End of input
        }

        input[bytes_read] = '\0'; // Null terminate the string

        // Parse input
        char *command = strtok(input, " \n"); // Extract first word as command
        char *arg1 = strtok(NULL, " \n");     // Extract first argument
        char *arg2 = strtok(NULL, " \n");     // Extract second argument
        char *arg3 = strtok(NULL, "\"\n");    // Extract third argument

        printf("command parent: %s\n", command);
        if (command != NULL)
        {
            printf("command parent if: %s\n", command);
            // printf("null deigl");

            if (strcmp(command, "exit") == 0 && arg1 == NULL)
            {
                exit(EXIT_SUCCESS);
            }
            if (strcmp(command, "gtuStudentGrades") == 0 && arg1 != NULL)
            {
                // printf("asdasd");
                // printf("Filename set to: %s\n", filename);
                gtuStudentGrades(arg1);
                continue;
            }

            pid_t pid = fork(); // Fork the process
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                // Child process
                if (filename[0] == '\0')
                {
                    printf("Please set a filename using 'gtuStudentGrades <filename>' before other commands.\n");
                }
                else if (strcmp(command, "addStudentGrade") == 0 && arg1 != NULL && arg2 != NULL)
                {
                    addStudentGrade(filename, arg1, arg2, arg3);
                }
                else if (strcmp(command, "searchStudent") == 0 && arg1 != NULL)
                {
                    searchStudent(filename, arg1);
                }
                else if (strcmp(command, "sortAll") == 0)
                {
                    printf("test");
                    sortAll(filename);
                }
                else if (strcmp(command, "showAll") == 0)
                {
                    showAll(filename, -1, -1);
                }
                else if (strcmp(command, "lstGrades") == 0)
                {
                    showAll(filename, 5, -1);
                }
                else if (strcmp(command, "lstSome") == 0 && arg1 != NULL && arg2 != NULL)
                {
                    int numEntries = atoi(arg1);
                    int pageNumber = atoi(arg2);
                    showAll(filename, numEntries, pageNumber);
                }
                else if (strcmp(command, "quit") == 0)
                {
                    exit(EXIT_SUCCESS); // Exit child process
                }
                else
                {
                    printf("Invalid command or arguments.\n");
                    exit(EXIT_SUCCESS); // Exit child process (error code
                }
                exit(EXIT_SUCCESS); // Exit child process
            }
            else
            {
                // Parent process
                int status;
                waitpid(pid, &status, 0); // Wait for child process to finish
            }
        }
    }

    exit(EXIT_SUCCESS); // Exit child process
}
void addStudentGrade(const char *filename, const char *name, const char *surname, const char *grade)
{
    printf("Adding student grade: %s, %s, %s\n", name, surname, grade);

    int fd = open(filename, O_WRONLY | O_APPEND); // Open the file in write-only mode and append mode
    if (fd == -1)
    {
        perror("open");
        return;
    }

    char buffer[MAX_INPUT_LENGTH];
    snprintf(buffer, sizeof(buffer), "%s %s %s\n", name, surname, grade); // Format the name and grade as a string
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));            // Write the string to the file
    if (bytes_written == -1)
    {
        perror("write");
    }

    close(fd); // Close the file
}

void searchStudent(const char *filename, const char *target)
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
            buffer[newline - buffer] = '\n';    // Restore newline character
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
            *newline = '\0';                   // Replace newline with null terminator
            char *name = strtok(buffer, " ");  // Extract name
            char *surname = strtok(NULL, " "); // Extract surname
            char *grade = strtok(NULL, " ");   // Extract grade

            lines[numLines] = strdup(buffer); // Allocate memory and store the line in the array
            numLines++;

            buffer[newline - buffer] = '\n';    // Restore newline character
            strcpy(buffer, newline + 1);        // Update buffer to start from the new newline pointer
            newline = strchr(buffer + 1, '\n'); // Find the next newline character

            // for taking last string of the document. could be deleted
            if (newline == NULL || *newline == '\0')
            {
                lines[numLines] = strdup(buffer);
                numLines++;
                break;
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

    // Sort the lines in ascending order based on names
    for (int i = 0; i < numLines - 1; i++)
    {
        for (int j = 0; j < numLines - i - 1; j++)
        {
            char *name1 = strtok(lines[j], " ");
            char *name2 = strtok(lines[j + 1], " ");

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
        printf("%s\n", lines[i]);
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

void showAll(const char *filename, int numEntries, int pageNumber)
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
    int ct = 0;

    printf("Displaying all student grades\n");
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        newline = strchr(buffer, '\n'); // Find the newline character
        // printf("%s, buffer", buffer);

        while (newline != NULL)
        {
            int printFlag = 1;
            if (ct == 5 && numEntries == 5 && pageNumber == -1)
            {
                exit(EXIT_SUCCESS);
            }

            else if (numEntries != -1 && pageNumber != -1 && ((ct > (pageNumber * numEntries) - 1) || ct < ((pageNumber - 1) * numEntries)))
            {
                printFlag = 0;
            }
            *newline = '\0'; // Replace newline with null terminator
            if (printFlag == 1)
                printf("Record %d: %s\n", ct, buffer);

            buffer[newline - buffer] = '\n';    // Restore newline character
            strcpy(buffer, newline + 1);        // Update buffer to start from the new newline pointer
            newline = strchr(buffer + 1, '\n'); // Find the next newline character
            ct++;

            // for taking last string of the document. could be deleted
            if ((newline == NULL || *newline == '\0') && printFlag == 1)
            {
                printf("Record %d: %s\n", ct, buffer);
                break;
            }
            // Print name, surname and grade
        }

        lseek(fd, newline - buffer + 1, SEEK_CUR); // Move the file pointer to the next line
        strcpy(buffer, "");                        // Clear the buffer
    }
}

void gtuStudentGrades(const char *file)
{
    int fd = open(file, O_WRONLY | O_CREAT | O_EXCL, 0644); // Open the file in write-only mode, create if not exists, and fail if already exists
    if (fd == -1)
    {
        strcpy(filename, file);
        perror("open");
        return;
    }

    strcpy(filename, file);
    printf("Filename set to: %s\n", filename);

    close(fd); // Close the file
}