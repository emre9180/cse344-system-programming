#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "child2.h"

void child2_function() {
    // Open the second named pipe for reading
    int fd2 = open("/home/emre/Desktop/ödev2/fifo2", O_RDONLY);
    if (fd2 == -1) {
        perror("Error opening fifo2 for reading");
        exit(1); // Exiting because this is a fatal error for this child process
    }

    int result;
    if (read(fd2, &result, sizeof(result)) == -1) {
        perror("Error reading from fifo2");
        close(fd2);
        exit(1); // Exiting because this is a fatal error for this child process
    }

    // Close the second named pipe after reading
    close(fd2);

    printf("Sum of the numbers: %d\n", result);
    sleep(2);
    // Exiting normally indicating successful operation
    exit(EXIT_SUCCESS);
}
