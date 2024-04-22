#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "common.h"
#include "client_info_queue.h"

char server_req_fifo[256];
char client_res_fifo[256];
int fd_client_cmd = -1;
int fd_client_res = -1;
void sigint_handler(int signum)
{
    // Close file descriptors
    if (fd_client_cmd != -1)
    {
        close(fd_client_cmd);
    }
    if (fd_client_res != -1)
    {
        close(fd_client_res);
    }

    // Unlink fifos
    unlink(server_req_fifo);
    unlink(client_res_fifo);

    // Exit the program
    exit(EXIT_SUCCESS);
}

void write_to_server_fifo(int server_pid, int child_pid)
{
    char server_fifo[SERVER_FIFO_LEN];
    sprintf(server_fifo, SERVER_FIFO, server_pid);
    printf("Server FIFO: %s\n", server_fifo);

    // Write arguments and process id to SERVER_FIFO
    int fd_server_reg = open(server_fifo, O_WRONLY);
    if (fd_server_reg == -1)
    {
        perror("Failed to open SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    struct client_info cli_info;
    cli_info.pid = child_pid;
    cli_info.wait = 0;
    getcwd(cli_info.cwd, CWD_SIZE);

    ssize_t bytes_written = write(fd_server_reg, &cli_info, sizeof(struct client_info));
    if (bytes_written == -1)
    {
        perror("Failed to write to fd_server_reg");
        exit(EXIT_FAILURE);
    }

    close(fd_server_reg);
    printf("Arguments and process id written to SERVER_FIFO.\n");
}

int main(int argc, char *argv[])
{
    int shm_fd;
    void *shm_addr;
    Semaphores *sems;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Connect/tryConnect> <ServerPID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for SIGINT (Ctrl+C) using sigaction
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Failed to set up signal handler for SIGINT");
        exit(EXIT_FAILURE);
    }

    const char *mode = argv[1];
    int server_pid = atoi(argv[2]);
    int child_pid = getpid();

    open_shared_memory(&shm_fd, &shm_addr);
    write_to_server_fifo(server_pid, child_pid);

    // Get the pointer to the structured semaphores
    // sems = (Semaphores *)shm_addr;
    // sem_post(&sems->full);

    sprintf(server_req_fifo, SERVER_REQ_FIFO, child_pid);
    create_named_pipe(server_req_fifo);

    sprintf(client_res_fifo, CLIENT_RES_FIFO, child_pid);
    create_named_pipe(client_res_fifo);
    printf("asdf\n  %s\n", client_res_fifo);
    // Open client response FIFO in read mode
    fd_client_res = open(client_res_fifo, O_RDONLY);
    if (fd_client_res == -1)
    {
        perror("Failed to open client response FIFO");
        exit(EXIT_FAILURE);
    }

    // Read 1 byte from parent
    char read_byte;
    ssize_t bytes_read = read(fd_client_res, &read_byte, sizeof(char));
    if (bytes_read == -1)
    {
        perror("Failed to read from parent");
        exit(EXIT_FAILURE);
    }

    // Check if the read byte is integer "1"
    if (read_byte == '1')
    {
        printf("Connection is successful.\n");
    }

    while (1)
    {
        // Open client command FIFO in write mode
        fd_client_cmd = open(server_req_fifo, O_WRONLY);
        if (fd_client_cmd == -1)
        {
            perror("Failed to open client command FIFO");
            exit(EXIT_FAILURE);
        }

        // Print the command prompt
        printf("Enter command: ");

        // Read the command from user
        char command[256];
        fgets(command, sizeof(command), stdin);

        // Write the command to the client command FIFO
        ssize_t bytes_written_cmd = write(fd_client_cmd, command, strlen(command));
        if (bytes_written_cmd == -1)
        {
            perror("Failed to write command to client command FIFO");
            exit(EXIT_FAILURE);
        }
    }

    // Cleanup: Do not destroy semaphores here, just unmap and close
    munmap(shm_addr, SHM_SIZE);
    close(shm_fd);
    close(fd_client_cmd);
    close(fd_client_res);
    unlink(server_req_fifo);
    unlink(client_res_fifo);

    return 0;
}
