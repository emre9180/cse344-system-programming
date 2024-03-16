#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void openFile(const char* filename) {
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (fd < 0) {
        perror("Open failed");
        exit(EXIT_FAILURE);
    }

    const char* data = "Sample data";
    ssize_t bytes_written = write(fd, data, strlen(data));
    if (bytes_written < 0) {
        perror("Write failed");
        exit(EXIT_FAILURE);
    }

    close(fd);
}
