#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "include/threads.h"

// Manager thread function
void *manager_thread(void *arg)
{
    const char **args = (const char **)arg;
    const char *source_dir = args[3];
    const char *destination_dir = args[4];
    const int num_workers = atoi(args[2]);
    pthread_barrier_wait(&barrier); // Wait at the barrier for all worker threads to finish
    
    process_directory(source_dir, destination_dir, num_workers);

    // Signal that all files have been added to the buffer. Worker threads will exit when "done" variable is set to 1.
    pthread_mutex_lock(&buffer_mutex);         // Lock the buffer mutex
    done = 1;                                  // Set done flag. Worker threads will exit when done flag is set to 1
    pthread_cond_broadcast(&buffer_not_empty); // Signal that buffer is not empty for worker threads
    pthread_mutex_unlock(&buffer_mutex);       // Unlock the buffer mutex

    // pthread_barrier_wait(&barrier); // Wait at the barrier for all worker threads to finish

    pthread_exit(NULL);
}

// Worker thread function
void *worker_thread()
{
    pthread_mutex_lock(&buffer_mutex);   // Lock the buffer mutex
    active_threads++;                    // Increment active threads count
    pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex
    
    pthread_barrier_wait(&barrier); // Wait at the barrier for all worker threads to finish

    while (1)
    {
        pthread_mutex_lock(&buffer_mutex); // Lock the buffer mutex

        // if(closed_fd >= 500) {
        //     // printf("Total closed files and buffer index: %d - %d\n", closed_fd, buffer_index);
        //     pthread_barrier_wait(&barrier);
        // }

        while (buffer_index == 0 && !done) // Wait for buffer to become not empty
        {
            pthread_cond_wait(&buffer_not_empty, &buffer_mutex); // Wait for buffer to become not empty
        }

        if (buffer_index == 0 && done) // Check if all files have been copied
        {
            pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex
            break;             // Exit the thread
        }

        // Get file info from the buffer
        file_info_t file_info = buffer[--buffer_index]; // Get file info from buffer
        pthread_cond_signal(&buffer_not_full);          // Signal that buffer is not full for manager thread
        pthread_mutex_unlock(&buffer_mutex);            // Unlock the buffer mutex

        // Copy the file
        copy_file(&file_info); // Copy the file

        // Close file descriptors
        close(file_info.src_fd);
        close(file_info.dst_fd);

        pthread_mutex_lock(&buffer_mutex); // Lock the buffer mutex
        closed_fd++;
        pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex

        // pthread_barrier_wait(&barrier); // Wait at the barrier for all threads to reach this point

    }

    pthread_mutex_lock(&buffer_mutex);   // Lock the buffer mutex
    active_threads--;                    // Decrement active threads count
    pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex

    pthread_exit(NULL);
}

// Function to copy a file
void copy_file(file_info_t *file_info)
{
    char buffer[4096];
    ssize_t bytes_read, bytes_written;
    ssize_t total_written = 0; // Track the total bytes written for this file

    // Read from source file and write to destination file
    while ((bytes_read = read(file_info->src_fd, buffer, 4096)) > 0)
    {
        bytes_written = write(file_info->dst_fd, buffer, bytes_read);
        if (bytes_written == -1)
        {
            perror("write");
            break;
        }
        total_written += bytes_written;
    }

    if (bytes_read == -1)
    {
        perror("read");
    }

    // Update files copied count
    pthread_mutex_lock(&buffer_mutex);
    files_copied++;
    total_bytes_copied += total_written;
    // printf("Copied: %s -> %s\n", file_info->src_path, file_info->dst_path);
    pthread_mutex_unlock(&buffer_mutex);
}

void process_directory(const char *source_dir, const char *destination_dir, int num_workers) {
    struct dirent *entry;
    DIR *dp = opendir(source_dir);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    if (mkdir(destination_dir, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        closedir(dp);
        return;
    }

    pthread_mutex_lock(&buffer_mutex);
    directory_copied++;
    pthread_mutex_unlock(&buffer_mutex);

    while ((entry = readdir(dp)) != NULL) {
        char src_path[MAX_BUFFER_SIZE];
        char dst_path[MAX_BUFFER_SIZE];
        snprintf(src_path, sizeof(src_path), "%s/%s", source_dir, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", destination_dir, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            process_directory(src_path, dst_path, num_workers);
        } else if (entry->d_type == DT_REG || entry->d_type == DT_FIFO) {
            file_info_t file_info;
            strncpy(file_info.src_path, src_path, MAX_BUFFER_SIZE);
            strncpy(file_info.dst_path, dst_path, MAX_BUFFER_SIZE);

            int openFlags = O_RDONLY;
            if (entry->d_type == DT_FIFO) {
                openFlags |= O_NONBLOCK; // Open FIFOs in non-blocking mode to avoid hanging
            }

             file_info.src_fd = open(file_info.src_path, openFlags);
            if (file_info.src_fd == -1) {
                perror("open source file");
                continue;
            }

            // // If the current fd is equal to the maximum fd, then we have reached the maximum number of file descriptors. Wait for barrier, after that reset the current fd to 0. Then, reinitiliaze the barrier
            // if(current_fd >= 500) {
            //     // printf("Total files: %d\n", current_fd);
            //     pthread_barrier_wait(&barrier);
            //     pthread_mutex_lock(&buffer_mutex);
            //     current_fd = 0;
            //     closed_fd = 0;
            //     pthread_barrier_destroy(&barrier);
            //     if (pthread_barrier_init(&barrier, NULL, 2) != 0) { // +1 for the manager thread
            //         perror("Failed to initialize barrier");
            //         exit(EXIT_FAILURE);
            //     }
            //     pthread_mutex_unlock(&buffer_mutex);
                
            // }

            file_info.dst_fd = open(file_info.dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_info.dst_fd == -1) {
                perror("open destination file");
                close(file_info.src_fd);
                continue;
            }

            // Check if the file is a FIFO file
            struct stat st;
            if (fstat(file_info.src_fd, &st) == 0 && S_ISFIFO(st.st_mode))
            {
                pthread_mutex_lock(&buffer_mutex);
                fifo_files_copied++;
                pthread_mutex_unlock(&buffer_mutex);
            }
            else
            {
                pthread_mutex_lock(&buffer_mutex);
                regular_files_copied++;
                pthread_mutex_unlock(&buffer_mutex);
            }

            current_fd++;

            pthread_mutex_lock(&buffer_mutex);

            while (buffer_index == buffer_size) {
                pthread_cond_wait(&buffer_not_full, &buffer_mutex);
            }

            buffer[buffer_index++] = file_info;
            pthread_cond_signal(&buffer_not_empty);
            pthread_mutex_unlock(&buffer_mutex);
        }
    }

    closedir(dp);
}