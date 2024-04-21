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



void write_to_server_fifo(int server_pid, int child_pid) {
    char server_fifo[SERVER_FIFO_LEN];
    sprintf(server_fifo, SERVER_FIFO, server_pid);
    printf("Server FIFO: %s\n", server_fifo);

    // Write arguments and process id to SERVER_FIFO
    int fd_server_reg = open(server_fifo, O_WRONLY);
    if (fd_server_reg == -1) {
        perror("Failed to open SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    struct client_info cli_info;
    cli_info.pid = child_pid;
    cli_info.wait = 0;
    getcwd(cli_info.cwd, CWD_SIZE);

    ssize_t bytes_written = write(fd_server_reg, &cli_info, sizeof(struct client_info));
    if (bytes_written == -1) {
        perror("Failed to write to fd_server_reg");
        exit(EXIT_FAILURE);
    }

    close(fd_server_reg);
    printf("Arguments and process id written to SERVER_FIFO.\n");
}

int main(int argc, char *argv[]) {
    int shm_fd;
    void *shm_addr;
    Semaphores *sems;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Connect/tryConnect> <ServerPID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* mode = argv[1];
    int server_pid = atoi(argv[2]);
    int child_pid = getpid();

    open_shared_memory(&shm_fd, &shm_addr);
    write_to_server_fifo(server_pid, child_pid);

    // Get the pointer to the structured semaphores
    sems = (Semaphores *)shm_addr;
    sem_post(&sems->full);

    while(1) {
        // Your code here
    }

    // Cleanup: Do not destroy semaphores here, just unmap and close
    munmap(shm_addr, SHM_SIZE);
    close(shm_fd);

    return 0;
}
