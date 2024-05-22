#ifndef THREADS_H
#define THREADS_H

#define MAX_BUFFER_SIZE 1024

// Structure to hold file information
typedef struct
{
    int src_fd;                     // Source file descriptor
    int dst_fd;                     // Destination file descriptor
    char src_path[MAX_BUFFER_SIZE]; // Source file path
    char dst_path[MAX_BUFFER_SIZE]; // Destination file path
} file_info_t;

// External declarations for shared variables
extern file_info_t *buffer;             // Buffer to hold file_info_t objects
extern int buffer_size;                 // Size of the buffer
extern int buffer_index;                // Current index in the buffer
extern int done;                        // Flag to indicate completion
extern pthread_mutex_t buffer_mutex;    // Mutex for buffer access
extern pthread_cond_t buffer_not_full;  // Condition variable for buffer not full
extern pthread_cond_t buffer_not_empty; // Condition variable for buffer not empty
extern int files_copied;                // Number of files copied
extern int total_bytes_copied;          // Total bytes copied
extern int active_threads;              // Number of active threads

// Function prototypes
void *manager_thread(void *arg);        // Manager thread function
void *worker_thread(void *arg);         // Worker thread function
void copy_file(file_info_t *file_info); // Function to copy a file

#endif // THREADS_H
