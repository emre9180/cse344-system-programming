#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h> // Include the errno header
#include "child1.h"
#include "child2.h"

int child_counter = 0;
const int NUMBER_OF_CHILDREN = 2; // Total number of child processes

void sigchld_handler(int signum) {
    if (signum == SIGCHLD) {
        pid_t pid;
        int status;

        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("SIGCHLD received. Reaped child PID %d exited normally with status: %d\n", pid, WEXITSTATUS(status));
            child_counter++;
        }

        if (child_counter >= NUMBER_OF_CHILDREN) {
            printf("All child processes have terminated. Exiting...\n");
            unlink("/tmp/eyhw1_fifo1");
            unlink("/tmp/eyhw1_fifo2");
            exit(0);
        }
    } else if (signum == SIGINT) {
        printf("SIGINT caught, cleaning up FIFOs...\n");
        unlink("/tmp/eyhw1_fifo1");
        unlink("/tmp/eyhw1_fifo2");
        exit(0);
    }
}

void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGCHLD, &sa, NULL) == -1 || sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up sigaction");
        exit(1);
    }
}

void create_named_pipes() {
    // Create named pipes (FIFOs) if they do not exist
    const char* fifo_paths[2] = {"/tmp/eyhw1_fifo1", "/tmp/eyhw1_fifo2"};
    for (int i = 0; i < 2; i++) {
        if (access(fifo_paths[i], F_OK) == -1) {
            if (mkfifo(fifo_paths[i], S_IWUSR | S_IRUSR) == -1) {
                perror("Error creating fifo");
                exit(1);
            }
        }
    }
    printf("Two named pipes (FIFOs) created successfully.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // If there are not enough arguments provided, display an error message.
        printf("Be careful pls. Correct Usage of Command: %s <integer>\n", argv[0]);
        return 1;
    }

    // Convert the first argument to an integer
    int inputNumber = atoi(argv[1]);

    setup_signal_handler();
    create_named_pipes();

    // Fork the first child process and call child1_function
    pid_t child1_pid = fork();
    if (child1_pid == -1) 
    {
        perror("Error forking child1");
        unlink("/tmp/eyhw1_fifo1");
        unlink("/tmp/eyhw1_fifo2");
        exit(1);
    } 

    else if (child1_pid == 0) 
    {
         // Open the second named pipe for writing
        int fd2 = open("/tmp/eyhw1_fifo2", O_WRONLY);
        if (fd2 == -1) {
            perror("Error opening fifo2 for writing");
            exit(1); // Exiting because this is a fatal error for this child process
        }
        // Open the first named pipe for reading
        int fd1 = open("/tmp/eyhw1_fifo1", O_RDONLY);
        if (fd1 == -1) {
            perror("Error opening fifo1 for reading");
            exit(1); // Exiting because this is a fatal error for this child process
        }
        // Child process 1 logic
        sleep(10);
        child1_function(fd1, inputNumber, fd2);
    }

    else
    {
        // Fork the second child process and call child2_function
        pid_t child2_pid = fork();
        if (child2_pid == -1) {
            perror("Error forking child2");
            unlink("/tmp/eyhw1_fifo1");
            unlink("/tmp/eyhw1_fifo2");
            exit(1);
        } else if (child2_pid == 0) {
            // Child process 2 logic
            int fd2 = open("/tmp/eyhw1_fifo2", O_RDONLY);
            if (fd2 == -1) {
                perror("Error opening fifo2 for reading");
                exit(1); // Exiting because this is a fatal error for this child process
            }
            sleep(10);
            child2_function(fd2, inputNumber);
        }

        else
        {
            // Parent process logic
            // Open fifo1 in write mode
            int fifo1_fd = open("/tmp/eyhw1_fifo1", O_WRONLY);
            if (fifo1_fd == -1) 
            {
                perror("Error opening fifo1");
                unlink("/tmp/eyhw1_fifo1");
                unlink("/tmp/eyhw1_fifo2");
                exit(1);
            }

            // Open fifo2 in write mode
            int fifo2_fd = open("/tmp/eyhw1_fifo2", O_WRONLY);
            if (fifo2_fd == -1) 
            {
                perror("Error opening fifo1");
                unlink("/tmp/eyhw1_fifo1");
                unlink("/tmp/eyhw1_fifo2");
                exit(1);
            }

            const char* mult_string = "mult";
            if (write(fifo2_fd, mult_string, strlen(mult_string) + 1) == -1) 
            {
                perror("Error writing to fifo2");
                unlink("/tmp/eyhw1_fifo1");
                unlink("/tmp/eyhw1_fifo2");
                exit(1);
            }

            #include <time.h>
            srand(time(NULL)); // Seed the random number generator with the current time

            // Generate and write inputNumber random numbers to fifo1
            for (int i = 0; i < inputNumber; i++) 
            {
                int random_number = (rand() % inputNumber) + 1;
                if (write(fifo1_fd, &random_number, sizeof(int)) == -1) 
                {
                    perror("Error writing to fifo1");
                    unlink("/tmp/eyhw1_fifo1");
                    unlink("/tmp/eyhw1_fifo2");
                    exit(1);
                }

                if (write(fifo2_fd, &random_number, sizeof(int)) == -1) 
                {
                    perror("Error writing to fifo2");
                    unlink("/tmp/eyhw1_fifo1");
                    unlink("/tmp/eyhw1_fifo2");
                    exit(1);
                }
                
            }

            close(fifo2_fd);
            close(fifo1_fd);

            while (child_counter < 2) 
            {
                printf("Proceeding in parent...\n");
                sleep(2); // Pause and wait for signals
            }
            return 0;
        }   
    }
    
}
