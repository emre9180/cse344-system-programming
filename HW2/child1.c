#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "child1.h"

void child1_function() {
    // Open the first named pipe for reading
    int fd1 = open("/home/emre/Desktop/ödev2/fifo1", O_RDONLY);
    if (fd1 == -1) {
        perror("Error opening fifo1 for reading");
        exit(1); // Exiting because this is a fatal error for this child process
    }
    int numbers[10];
    if (read(fd1, numbers, sizeof(numbers)) == -1) {
        perror("Error reading from fifo1");
        close(fd1);
        exit(1); // Exiting because this is a fatal error for this child process
    }

    // Print the numbers
    printf("Numbers: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    // Close the first named pipe after reading
    close(fd1);

    // Calculate the sum of the numbers
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += numbers[i];
    }
    // Open the second named pipe for writing
    int fd2 = open("/home/emre/Desktop/ödev2/fifo2", O_WRONLY);
    if (fd2 == -1) {
        perror("Error opening fifo2 for writing");
        exit(1); // Exiting because this is a fatal error for this child process
    }

    if (write(fd2, &sum, sizeof(sum)) == -1) {
        perror("Error writing to fifo2");
        close(fd2);
        exit(1); // Exiting because this is a fatal error for this child process
    }

    // Close the second named pipe after writing
    close(fd2);
    sleep(4);
    exit(EXIT_SUCCESS);
}
