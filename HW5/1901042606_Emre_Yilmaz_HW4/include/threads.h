#ifndef THREADS_H
#define THREADS_H

#define MAX_BUFFER_SIZE 1024
#define MAX_FD 100

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
extern pthread_cond_t fd_limit;         // Condition variable for buffer not empty
extern pthread_barrier_t startBarrier;       // Barrier
extern pthread_barrier_t endBarrier;         // Barrier
extern int files_copied;                // Number of files copied
extern int total_bytes_copied;          // Total bytes copied
extern int active_threads;              // Number of active threads
extern int current_fd;                  // current opened file descriptor
extern int closed_fd;                   // closed file descriptor
extern int regular_files_copied;        // Number of regular files copied
extern int fifo_files_copied;           // Number of FIFO files copied
extern int directory_copied;            // Number of directories copied
extern int num_workers;                 // Number of worker threads
extern pthread_t manager_tid;           // Manager thread ID
extern pthread_t *worker_tids;          // Worker thread IDs
extern int sigint_received;             // Flag to indicate SIGINT signal received

// Function prototypes
void *manager_thread(void *arg);        // Manager thread function
void *worker_thread();         // Worker thread function
void copy_file(file_info_t *file_info); // Function to copy a file
void process_directory(const char *source_dir, const char *destination_dir, int num_workers); 

#endif // THREADS_H
