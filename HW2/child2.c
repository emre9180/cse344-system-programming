#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include "child2.h"

#define MAX_INTS 1024 // Adjust based on expected max number of integers
#define MAX_STR_LEN 1024 // Adjust based on expected max string length

void child2_function(int fd, int inputNumber) 
{
    int* numbers = (int*)malloc(inputNumber * sizeof(int));
    char str[MAX_STR_LEN];
    int numInts = 0;
    int tempInt;
    ssize_t bytesRead;

    // Read the string
    ssize_t totalBytesRead = 0;
    while ((bytesRead = read(fd, str + totalBytesRead, sizeof(char))) > 0) 
    {
        if (str[totalBytesRead] == '\0') 
        {
            break; // End of string
        }

        totalBytesRead += bytesRead;
        if (totalBytesRead >= MAX_STR_LEN - 1) 
        {
            fprintf(stderr, "String buffer capacity exceeded\n");
            str[MAX_STR_LEN - 1] = '\0'; // Ensure null-termination
            break;
        }
    }

    while ((bytesRead = read(fd, &tempInt, sizeof(tempInt))) > 0) 
    {
        if (numInts < inputNumber) {
            numbers[numInts] = tempInt;
        } 

        else 
        {
            fprintf(stderr, "Integer array capacity exceeded\n");
            close(fd);
            free(numbers);
            exit(EXIT_FAILURE);
        }

        numInts++;
        if (numInts >= inputNumber) 
        {
            break;
        }
    }

    if (bytesRead == -1) 
    {
        perror("Failed to read string from FIFO");
        close(fd);
        free(numbers);
        exit(EXIT_FAILURE);
    }

    bytesRead = read(fd, &tempInt, sizeof(tempInt));
    if(bytesRead > 0)
    {
        printf("(from CHILD-2) First fifo result is: %d\n", tempInt);
    }
    
    else
    {
        perror("Failed to read integers from FIFO");
        close(fd);
        free(numbers);
        exit(EXIT_FAILURE);
    }

    if(strcmp(str, "mult") == 0)
    {
        int result = 1;
        for(int i = 0; i < numInts; i++){
            result *= numbers[i];
        }
        printf("(from CHILD-2) Multiplication result: %d\n", result);
    }

    else if(strcmp(str, "sum") == 0)
    {
        double result = 0;
        for(int i = 0; i < numInts; i++)
        {
            result += numbers[i];
        }

        printf("(from CHILD-2) Sum result: %lf.3\n", result);
    }

    else
    {
        printf("Invalid operation\n");
        free(numbers);
        exit(EXIT_FAILURE);
    }


    // Output the read data
    printf("(from CHILD-2) Integers read: ");
    for (int i = 0; i < numInts; i++) 
    {
        printf("%d ", numbers[i]);
    }
    printf("\n(from CHILD-2) String read:%s\n", str);
    free(numbers);
    // Exiting normally indicating successful operation
    exit(EXIT_SUCCESS);
}
