#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void addStudent(const char* studentInfo, const char* grade) {
        int fd = open("test.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd == -1) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }

        size_t len_name = strlen(studentInfo);
        ssize_t bytesWritten = write(fd, studentInfo, len_name);

        if (bytesWritten == -1) {
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }

        size_t len_grade = strlen(grade);
        bytesWritten = write(fd, grade, len_grade);
        
        if (bytesWritten == -1) {
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }

        close(fd);
    }