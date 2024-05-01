#ifndef COMMON_H 
#define COMMON_H

#include <fcntl.h>          // File control options
#include <signal.h>         // Signal handling
#include <sys/mman.h>       // Memory management
#include <stdio.h>          // Standard I/O
#include <stdlib.h>         // Standard library
#include <unistd.h>         // POSIX API
#include <string.h>         // String operations
#include <sys/stat.h>       // File information
#include <sys/types.h>      // System data types
#include <sys/wait.h>       // Process management
#include <semaphore.h>      // Semaphore
#include <errno.h>          // Error handling
#include "common.h"         // Custom header
#include "../Sync/synch.h"  // Synchronization
#include "../Server/client_list.h"  // Client list

// Constants
#define EOF_COMMAND "EOF_IS_REACHED_05319346629"  // End-of-file command
#define MAX_CLIENTS 1  // Maximum number of clients
#define CWD_SIZE 256  // Size of current working directory
#define LOG_FILENAME "log.txt"  // Log file name

// Shared Memory
#define SHM_NAME "/my_shared_memory"  // Shared memory name
#define SHM_NAME_CLIENTS "/my_shared_memory_clients"  // Shared memory for clients

// FIFOs
#define SERVER_REQ_FIFO "/tmp/eymt_server_req_fifo_%d"  // Server request FIFO
#define CLIENT_RES_FIFO "/tmp/eymt_client_res_fifo_%d"  // Client response FIFO
#define SERVER_FIFO "/tmp/eymt_fifo_%d"  // Server FIFO
#define CLIENT_RES_FIFO_LEN 256  // Length of client response FIFO
#define SERVER_FIFO_LEN 256  // Length of server FIFO

// Semaphore
#define SEMAPHORE_NAME "/eymt_semaphore"  // Semaphore name

// Structures
typedef struct 
{
    char file[256];  // File name
    int line_number;  // Line number
} readF_command;  // Read file command

typedef struct 
{
    char file[256];  // File name
    int line_number;  // Line number
    char string[256];  // String to write
} writeF_command;  // Write file command

typedef struct 
{
    char file[256];  // File name
} upload_command;  // Upload file command

typedef struct 
{
    char file[256]; // File name
} download_command;  // Download file command

typedef struct 
{
    char file[256];  // File name
} arch_command;  // Archive file command

struct request_header {
    pid_t pid;  // Process ID
    size_t data_size;  // Data size
};

struct response_header {
    size_t data_size;  // Data size
};

/*
* Function to create a named pipe (FIFO)
* @param server_fifo The name of the server FIFO
*/
void create_named_pipe(const char* server_fifo);

#endif