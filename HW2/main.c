#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h> // Include the errno header
#include "child1.h"
#include "child2.h"

int child_counter = 0;
const int NUMBER_OF_CHILDREN = 2; // Total number of child processes

void sigchld_handler(int signum) {
    pid_t pid;
    int status;
    child_counter++;
    // printf("a\n");
    //pid = waitpid(-1, &status, WNOHANG);
    
    // while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    //     printf("Reaped child PID: %d\n", pid);
    //     child_counter++;
    // }

    // if (pid == 0) {
    //     printf("No more terminated children to reap at this moment.\n");
    // } else if (pid == -1) {
    //     perror("waitpid error");
    // }

    if (child_counter >= NUMBER_OF_CHILDREN) {
        // Cleanup code if necessary
        unlink("/home/emre/Desktop/ödev2/fifo1");
        unlink("/home/emre/Desktop/ödev2/fifo2");
        exit(0);
    }
}

void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;


    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error setting up sigaction");
        exit(1);
    }
}

void create_named_pipes() {
    // Create named pipes (FIFOs) if they do not exist
    const char* fifo_paths[2] = {"/home/emre/Desktop/ödev2/fifo1", "/home/emre/Desktop/ödev2/fifo2"};
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

int main() {
    setup_signal_handler();

    // Fork the first child process and call child1_function
    pid_t child1_pid = fork();
    if (child1_pid == -1) {
        perror("Error forking child1");
        exit(1);
    } else if (child1_pid == 0) {
        sleep(2); // Sleep for 10 seconds to simulate a long-running process
        // Child process 1 logic
        child1_function();
    }

    else{
        // Fork the second child process and call child2_function
        pid_t child2_pid = fork();
        if (child2_pid == -1) {
            perror("Error forking child2");
            exit(1);
        } else if (child2_pid == 0) {
            // Child process 2 logic
            sleep(2); // Sleep for 10 seconds to simulate a long-running process
            child2_function();
        }

        else
        {
            create_named_pipes();
            // Parent process
            // Wait for both child processes to finish
            

            // Open fifo1 in write mode
            int fifo1_fd = open("/home/emre/Desktop/ödev2/fifo1", O_WRONLY);
            if (fifo1_fd == -1) {
                perror("Error opening fifo1");
                exit(1);
            }

            // Generate and write 10 random numbers to fifo1
            for (int i = 0; i < 10; i++) {
                int random_number = rand() % 10;
                if (write(fifo1_fd, &random_number, sizeof(int)) == -1) {
                    perror("Error writing to fifo1");
                    exit(1);
                }
            }

            close(fifo1_fd);
            while (wait(NULL) != -1 || errno != ECHILD) {
                // Optionally handle EINTR if wait was interrupted by a signal handler
                if (errno == EINTR) {
                    continue;
                }
            }
            while (child_counter < 2) {
                pause(); // Pause and wait for signals
            }
            return 0;
        }   
    }
    
}
