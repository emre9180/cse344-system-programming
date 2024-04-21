#ifndef COMMON_H 
#define COMMON_H


#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include "common.h"


#define MAX_CLIENTS 1

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE sizeof(sem_t)  // Adjust size as needed for more shared data
#define SERVER_REQ_FIFO "/tmp/eymt_server_req_fifo_%d"
#define CLIENT_RES_FIFO "/tmp/eymt_client_res_fifo_%d"
#define SERVER_FIFO "/tmp/eymt_fifo_%d"
#define SEMAPHORE_NAME "/eymt_semaphore"

#define CLIENT_RES_FIFO_LEN 256
#define SERVER_FIFO_LEN 256

#define CWD_SIZE 256



typedef struct {
    sem_t mutex;
    sem_t empty;
    sem_t full;
} Semaphores;

struct request_header {
    pid_t pid;
    size_t data_size;
};

struct response_header {
    size_t data_size;
};

// Function to create a named pipe
void create_named_pipe(const char* server_fifo);

// Function to initialize shared memory
Semaphores *initialize_shared_memory();

// Function to cleanup shared memory
void cleanup_shared_memory(Semaphores* sems);

// Function to cleanup child processes
void cleanup_child_processes(Semaphores* sems);

// Function to open shared memory
void open_shared_memory(int *shm_fd, void **shm_addr);


#endif