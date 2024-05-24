#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include "include/threads.h"

#define MAX_BUFFER_SIZE 1024 // Maximum buffer size

// Shared buffer for file descriptors
file_info_t *buffer;    // Buffer to hold file_info_t objects
int buffer_size;        // Size of the buffer
int buffer_index = 0;   // Current index in the buffer
int done = 0;           // Flag to indicate completion
int active_threads = 0; // Number of active threads

// Mutex and condition variables
pthread_mutex_t buffer_mutex;    // Mutex for buffer access
pthread_cond_t buffer_not_full;  // Condition variable for buffer not full
pthread_cond_t buffer_not_empty; // Condition variable for buffer not empty

// Barrier
pthread_barrier_t barrier;

// Statistics
int files_copied = 0;       // Number of files copied
int total_bytes_copied = 0; // Total bytes copied
int current_fd = 0;
int closed_fd = 0;

int regular_files_copied = 0;
int fifo_files_copied = 0;
int directory_copied = 0;

// Function prototypes
void print_usage(const char *program_name);
void signal_handler(int sig);

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse command-line arguments
    buffer_size = atoi(argv[1]);
    int num_workers = atoi(argv[2]);
    // const char *source_dir = argv[3];
    // const char *destination_dir = argv[4];

    // Allocate buffer
    buffer = (file_info_t *)malloc(buffer_size * sizeof(file_info_t));
    if (!buffer)
    {
        perror("Failed to allocate buffer");
        exit(EXIT_FAILURE);
    }

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&buffer_mutex, NULL) != 0)
    {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&buffer_not_full, NULL) != 0)
    {
        perror("Failed to initialize condition variable buffer_not_full");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&buffer_not_empty, NULL) != 0)
    {
        perror("Failed to initialize condition variable buffer_not_empty");
        exit(EXIT_FAILURE);
    }
    if (pthread_barrier_init(&barrier, NULL, num_workers + 1) != 0) { // +1 for the manager thread
        perror("Failed to initialize barrier");
        exit(EXIT_FAILURE);
    }

    // Initialize worker threads
    pthread_t manager_tid;
    pthread_t worker_tids[num_workers];

    // Register signal handler using sigaction
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Failed to set up signal handler");
        exit(EXIT_FAILURE);
    }

    // Start measuring execution time
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Start the manager thread
    if (pthread_create(&manager_tid, NULL, manager_thread, (void *)argv) != 0)
    {
        perror("Failed to create manager thread");
        exit(EXIT_FAILURE);
    }

    // Start the worker threads
    for (int i = 0; i < num_workers; i++)
    {
        if (pthread_create(&worker_tids[i], NULL, worker_thread, NULL) != 0)
        {
            perror("Failed to create worker thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for the manager thread to complete
    pthread_join(manager_tid, NULL);

    // Wait for the worker threads to complete
    for (int i = 0; i < num_workers; i++)
    {
        pthread_join(worker_tids[i], NULL);
    }


    // End measuring execution time
    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // Print statistics
    printf("Total bytes copied: %d\n", total_bytes_copied);
    printf("Total files copied: %d\n", files_copied);
    printf("Total regular files copied: %d\n", regular_files_copied);
    printf("Total FIFO files copied: %d\n", fifo_files_copied);
    printf("Total directories copied: %d\n", directory_copied);
    printf("Execution time: %f seconds\n", elapsed_time);

    // Clean up
    free(buffer);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);
    pthread_barrier_destroy(&barrier);

    return 0;
}

// Function to print usage information
void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <buffer_size> <num_workers> <source_dir> <destination_dir>\n", program_name);
}

// Signal handler function to set the done flag when SIGINT is received
void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        pthread_mutex_lock(&buffer_mutex);
        done = 1;
        pthread_cond_broadcast(&buffer_not_empty);
        pthread_mutex_unlock(&buffer_mutex);
        exit(EXIT_SUCCESS);
    }
}