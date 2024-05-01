#include "server_util.h"
#include "../Sync/synch.h"
struct client_list *client_list;


// Function to cleanup child processes
// This function waits for child processes to terminate and posts to the empty semaphore of the dir_syncs structure.
// It uses waitpid() with WNOHANG option to non-blockingly wait for child processes.
// Once a child process terminates, it posts to the empty semaphore to indicate that a slot is available for a new child process.

void cleanup_child_processes(struct dir_sync *dir_syncs) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        sem_post(&dir_syncs->sems.empty);
    }
}

// Function to cleanup resources
void cleanup(int server_fd, int client_res_fd, int clienet_req_fd, struct dir_sync *dir_syncs)
{
    // Close the safe directory
    closeSafeDir(dir_syncs);

    // Remove the server FIFO
    char server_fifo[256];
    sprintf(server_fifo, SERVER_FIFO, getppid());
    unlink(server_fifo);

    // Close the server file descriptor
    if (server_fd != -1)
    {
        close(server_fd);
    }

    // Close the client response file descriptor
    if (client_res_fd != -1)
    {
        close(client_res_fd);
    }

    // Close the client request file descriptor
    if (clienet_req_fd != -1)
    {
        close(clienet_req_fd);
    }

    if (shm_unlink(SHM_NAME) == -1) {
        perror("Error unlinking shared memory segment");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(SHM_NAME_CLIENTS) == -1) {
        perror("Error unlinking shared memory segment for clients");
        exit(EXIT_FAILURE);
    }
}

// Function to create a shared memory segment and map it to the process's address space
void create_shared_memory(const char *shm_name, void **shm_ptr, int shm_size)
{
    int shm_fd;

    /* Create a shared memory segment */
    if ((shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("Error creating shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Set the size of the shared memory segment */
    if (ftruncate(shm_fd, shm_size) == -1)
    {
        perror("Error setting size of shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Map the shared memory segment to the process's address space */
    if ((*shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
    {
        perror("Error mapping shared memory segment");
        exit(EXIT_FAILURE);
    }
}


void write_log_file(char *message, struct dir_sync *sync_dir)
{
    time_t current_time;
    char time_string[50];
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info);
    enterRegionWriter(getSafeFile(sync_dir, LOG_FILENAME));
    FILE *log_file = fopen(LOG_FILENAME, "a");
    if (log_file == NULL)
    {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    fprintf(log_file, "in here include time: [%s] %s", time_string, message);
    fclose(log_file);
    exitRegionWriter(getSafeFile(sync_dir, LOG_FILENAME));
}