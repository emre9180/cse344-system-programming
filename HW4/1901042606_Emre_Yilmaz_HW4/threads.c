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

     // Initialize mutex and condition variables
    if (pthread_mutex_init(&buffer_mutex, NULL) != 0)
    {
        perror("Failed to initialize mutex");
        free(buffer);
        free(worker_tids);
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&buffer_not_full, NULL) != 0)
    {
        perror("Failed to initialize condition variable buffer_not_full");
        free(buffer);
        free(worker_tids);
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&buffer_not_empty, NULL) != 0)
    {
        perror("Failed to initialize condition variable buffer_not_empty");
        free(buffer);
        free(worker_tids);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&fd_limit, NULL) != 0)
    {
        perror("Failed to initialize condition variable buffer_not_empty");
        free(buffer);
        free(worker_tids);
        exit(EXIT_FAILURE);
    }
    
    pthread_barrier_wait(&startBarrier); // Wait at the barrier for all worker threads to finish
    process_directory(source_dir, destination_dir, num_workers);

    // After, copying process is done

    // Signal that all files have been added to the buffer. Worker threads will exit when "done" variable is set to 1.
    pthread_mutex_lock(&buffer_mutex);         // Lock the buffer mutex
    done = 1;                                  // Set done flag. Worker threads will exit when done flag is set to 1
    pthread_cond_broadcast(&buffer_not_empty); // Signal that buffer is not empty for worker threads
    pthread_mutex_unlock(&buffer_mutex);       // Unlock the buffer mutex

    pthread_barrier_wait(&endBarrier); // Wait at the barrier for all worker threads to finish
    pthread_mutex_destroy(&buffer_mutex); // Destroy the buffer mutex
    pthread_cond_destroy(&buffer_not_full); // Destroy the buffer not full condition variable
    pthread_cond_destroy(&buffer_not_empty); // Destroy the buffer not empty condition variable

    return (void *) 0;
}

// Worker thread function
void *worker_thread()
{   
    pthread_barrier_wait(&startBarrier); // Wait at the barrier for all worker threads to finish

    pthread_mutex_lock(&buffer_mutex);   // Lock the buffer mutex
    active_threads++;                    // Increment active threads count
    pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex

    while (1)
    {
        pthread_mutex_lock(&buffer_mutex); // Lock the buffer mutex

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
        current_fd--;
        pthread_cond_signal(&fd_limit); // Signal that buffer is not full for manager thread
        pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex
    }

    pthread_mutex_lock(&buffer_mutex);   // Lock the buffer mutex
    active_threads--;                    // Decrement active threads count
    pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex
    pthread_barrier_wait(&endBarrier); // Wait at the barrier for all threads to reach this point
    return (void *) 0;
}
// Function to copy a file
void copy_file(file_info_t *file_info)
{
    char buffer[4096];
    ssize_t bytes_read;
    ssize_t total_written = 0; // Track the total bytes written for this file

    // Read from source file and write to destination file
    while ((bytes_read = read(file_info->src_fd, buffer, sizeof(buffer))) > 0)
    {
        ssize_t bytes_written = 0;
        ssize_t bytes_to_write = bytes_read;

        // Ensure all bytes read are written
        while (bytes_to_write > 0)
        {
            ssize_t result = write(file_info->dst_fd, buffer + bytes_written, bytes_to_write);
            if (result == -1)
            {
                perror("write");
                break;
            }
            bytes_written += result;
            bytes_to_write -= result;
        }

        total_written += bytes_written;

        // If there was a write error, break out of the loop
        if (bytes_to_write > 0)
        {
            break;
        }
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
    if(sigint_received)
    {
        return ;
    }
        
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

        if(sigint_received)
        {
            return;
        }

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

            

            pthread_mutex_lock(&buffer_mutex);
            current_fd++;

            while (buffer_index == buffer_size && !done) {
                pthread_cond_wait(&buffer_not_full, &buffer_mutex);
            }

            while (current_fd == 500 && !done) {
                pthread_cond_wait(&fd_limit, &buffer_mutex);

            }

            buffer[buffer_index++] = file_info;
            pthread_cond_signal(&buffer_not_empty);
            pthread_mutex_unlock(&buffer_mutex);
        }
    }

    closedir(dp);
}