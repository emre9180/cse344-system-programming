#include "server_util.h"

// Function to cleanup child processes
void cleanup_child_processes(struct dir_sync *dir_syncs) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        sem_post(&dir_syncs->sems.empty);
    }
}

// Function to cleanup server resources
void cleanup_server(int server_fd, int client_res_fd, int server_req_fd, struct dir_sync *dir_syncs, int parent_pid) {
    closeSafeDir(dir_syncs);
    char server_fifo[256];
    sprintf(server_fifo, SERVER_FIFO, parent_pid);
    printf("Unlinking server FIFO: %s\n", server_fifo);
    if (unlink(server_fifo) == -1) {
        perror("Error unlinking server FIFO");
    }

    if (server_fd != -1) {
        close(server_fd);
    }

    if (client_res_fd != -1) {
        close(client_res_fd);
    }

    if (server_req_fd != -1) {
        close(server_req_fd);
    }
}
