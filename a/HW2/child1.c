#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "child1.h"

void child1_function(int fd1, int inputNumber, int fd2) {
    int tempInt;
    int numInts = 0;
    ssize_t bytesRead;

    int* numbers = (int*)malloc(inputNumber * sizeof(int));
    if (numbers == NULL) 
    {
        perror("Error allocating memory for numbers");
        exit(1); // Exiting because this is a fatal error for this child process
    }

    while ((bytesRead = read(fd1, &tempInt, sizeof(tempInt))) > 0) 
    {
        if (numInts < inputNumber) {
            numbers[numInts] = tempInt;
        } 

        else 
        {
            fprintf(stderr, "Integer array capacity exceeded\n");
            close(fd1);
            free(numbers);
            exit(EXIT_FAILURE);
        }

        numInts++;
        if (numInts >= inputNumber) 
        { // Check for \0 indicating end of integers
            break;
        }
    }

    if (bytesRead == -1) 
    {
        perror("Failed to read string from FIFO");
        close(fd1);
        free(numbers);
        exit(EXIT_FAILURE);
    }

    // Print the numbers
    printf("(from CHILD-1) Numbers: ");
    for (int i = 0; i < inputNumber; i++) 
    {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    // Calculate the sum of the numbers
    int sum = 0;
    for (int i = 0; i < inputNumber; i++) 
    {
        sum += numbers[i];
    }
    free(numbers);

    ssize_t bytesWritten;

    bytesWritten = write(fd2, &sum, sizeof(sum));
    if (bytesWritten == -1) 
    {
        // An error occurred during the write operation
        perror("Error writing to fifo2");
        close(fd2);
        exit(1); // Exiting because this is a fatal error for this child process
    } 
    
    else if (bytesWritten != sizeof(sum)) 
    {
        // Not all bytes were written
        fprintf(stderr, "Incomplete write to fifo2\n");
        close(fd2);
        exit(1); // Exiting because this is a critical issue for this child process
    }

    else
        printf("(from CHILD-1) Sum: %d\n", sum);
    

    // Close the second named pipe after writing
    close(fd1);
    close(fd2);
    exit(EXIT_SUCCESS);
}
