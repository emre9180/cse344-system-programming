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
void addStudentGrade(const char *filename, const char *name, const char *grade);
void searchStudent(const char *filename, const char *name);
void sortAll(const char *filename);
void showAll(const char *filename);
void lstGrades(const char *filename);
void lstSome(const char *filename, int numEntries, int pageNumber);
void gtuStudentGrades(const char *file);

int main() {
    char input[MAX_INPUT_LENGTH];
    strcpy(filename, "emre.txt");
    while (1) {
        ssize_t bytes_read = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        } else if (bytes_read == 0) {
            break; // End of input
        }

        input[bytes_read] = '\0'; // Null terminate the string

        // Parse input
        char *command = strtok(input, " \n"); // Extract first word as command
        char *arg1 = strtok(NULL, " \n"); // Extract first argument
        char *arg2 = strtok(NULL, "\"\n"); // Extract second argument

        if (command != NULL) {
            if (strcmp(command, "gtuStudentGrades") == 0 && arg1 != NULL) {
                    printf("asdasd");
                    // printf("Filename set to: %s\n", filename);
                    gtuStudentGrades(arg1);
                    continue;
            } 

            pid_t pid = fork(); // Fork the process
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                if (filename[0] == '\0') {
                    printf("Please set a filename using 'gtuStudentGrades <filename>' before other commands.\n");
                } else if (strcmp(command, "addStudentGrade") == 0 && arg1 != NULL && arg2 != NULL) {
                    addStudentGrade(filename, arg1, arg2);
                } else if (strcmp(command, "searchStudent") == 0 && arg1 != NULL) {
                    searchStudent(filename, arg1);
                } else if (strcmp(command, "sortAll") == 0) {
                    sortAll(filename);
                } else if (strcmp(command, "showAll") == 0) {
                    showAll(filename);
                } else if (strcmp(command, "lstGrades") == 0) {
                    lstGrades(filename);
                } else if (strcmp(command, "lstSome") == 0 && arg1 != NULL && arg2 != NULL) {
                    int numEntries = atoi(arg1);
                    int pageNumber = atoi(arg2);
                    lstSome(filename, numEntries, pageNumber);
                } else if (strcmp(command, "quit") == 0) {
                    exit(EXIT_SUCCESS); // Exit child process
                } else {
                    printf("Invalid command or arguments.\n");
                }
                exit(EXIT_SUCCESS); // Exit child process
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0); // Wait for child process to finish
            }
        }
    }

    return 0;
}
void addStudentGrade(const char *filename, const char *name, const char *grade) {
    printf("Adding student grade: %s, %s\n", name, grade);
    
    int fd = open(filename, O_WRONLY | O_APPEND); // Open the file in write-only mode and append mode
    if (fd == -1) {
        perror("open");
        return;
    }
    
    char buffer[MAX_INPUT_LENGTH];
    snprintf(buffer, sizeof(buffer), "%s %s\n", name, grade); // Format the name and grade as a string
    
    ssize_t bytes_written = write(fd, buffer, strlen(buffer)); // Write the string to the file
    if (bytes_written == -1) {
        perror("write");
    }
    
    close(fd); // Close the file
}



void searchStudent(const char *filename, const char *target) {
    printf("Searching for student: %s\n", target);
    
    int fd = open(filename, O_RDONLY); // Open the file in read-only mode
    if (fd == -1) {
        perror("open");
        return;
    }
    
    char buffer[1024];
    ssize_t bytes_read;
    char *newline;
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        newline = strchr(buffer, '\n'); // Find the newline character
        // printf("%s, buffer", buffer);
        
        while (newline != NULL) {
            *newline = '\0'; // Replace newline with null terminator
            
            char *name = strtok(buffer, " "); // Extract name
            char *surname = strtok(NULL, " "); // Extract surname
            char *grade = strtok(NULL, " "); // Extract grade
            // printf("bulunmadi: Name: %s, Surname: %s, Grade: %s\n", name, surname, grade);
            if (name != NULL && strcmp(name, target) == 0) {
                printf("The record has been found: Name: %s, Surname: %s, Grade: %s\n", name, surname, grade);
                break;
            }
            
            buffer[newline - buffer] = '\n'; // Restore newline character
            strcpy(buffer, newline + 1); // Update buffer to start from the new newline pointer
            newline = strchr(newline + 1, '\n'); // Find the next newline character
        }
        
        lseek(fd, newline - buffer + 1, SEEK_CUR); // Move the file pointer to the next line
        strcpy(buffer, ""); // Clear the buffer
    }
    
    if (bytes_read == -1) {
        perror("read");
    }
    
    close(fd); // Close the file
}

void sortAll(const char *filename) {
    printf("Sorting all student grades\n");
    // Implement sorting student grades in file using fork()
}

void showAll(const char *filename) {
    printf("Displaying all student grades\n");
    // Implement displaying all student grades in file using fork()
}

void lstGrades(const char *filename) {
    printf("Listing first 5 entries\n");
    // Implement listing first 5 entries in the file
}

void lstSome(const char *filename, int numEntries, int pageNumber) {
    printf("Listing entries between %d and %d\n", (pageNumber - 1) * numEntries + 1, pageNumber * numEntries);
    // Implement listing entries based on given page number and number of entries
}

void gtuStudentGrades(const char *file) {
    int fd = open(file, O_WRONLY | O_CREAT | O_EXCL, 0644); // Open the file in write-only mode, create if not exists, and fail if already exists
    if (fd == -1) {
        strcpy(filename, file);
        perror("open");
        return;
    }
    
    strcpy(filename, file);
    printf("Filename set to: %s\n", filename);
    
    close(fd); // Close the file
}