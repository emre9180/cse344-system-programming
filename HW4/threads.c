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
    struct dirent *entry;
    DIR *dp = opendir(source_dir);

    if (dp == NULL)
    {
        perror("opendir");
        pthread_exit(NULL);
    }

    // Iterate through the directory entries
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            file_info_t file_info; // File info structure

            // Set file info
            snprintf(file_info.src_path, sizeof(file_info.src_path), "%s/%s", source_dir, entry->d_name);
            snprintf(file_info.dst_path, sizeof(file_info.dst_path), "%s/%s", destination_dir, entry->d_name);

            // Open source file
            file_info.src_fd = open(file_info.src_path, O_RDONLY);
            if (file_info.src_fd == -1)
            {
                perror("open source file");
                continue;
            }

            // Open destination file. Create if it doesn't exist, truncate if it does.
            file_info.dst_fd = open(file_info.dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_info.dst_fd == -1)
            {
                perror("open destination file");
                close(file_info.src_fd);
                continue;
            }

            // Add file info to the buffer
            pthread_mutex_lock(&buffer_mutex);  // Lock the buffer mutex
            while (buffer_index == buffer_size) // Wait for buffer to become not full
            {
                pthread_cond_wait(&buffer_not_full, &buffer_mutex); // Wait for buffer to become not full
            }
            buffer[buffer_index++] = file_info;     // Add file info to buffer
            pthread_cond_signal(&buffer_not_empty); // Signal that buffer is not empty for worker threads
            pthread_mutex_unlock(&buffer_mutex);    // Unlock the buffer mutex
        }
    }

    closedir(dp);

    // Signal that all files have been added to the buffer. Worker threads will exit when "done" variable is set to 1.
    pthread_mutex_lock(&buffer_mutex);         // Lock the buffer mutex
    done = 1;                                  // Set done flag. Worker threads will exit when done flag is set to 1
    pthread_cond_broadcast(&buffer_not_empty); // Signal that buffer is not empty for worker threads
    pthread_mutex_unlock(&buffer_mutex);       // Unlock the buffer mutex

    pthread_exit(NULL);
}

// Worker thread function
void *worker_thread(void *arg)
{
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
            pthread_exit(NULL);                  // Exit the thread
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
    }

    pthread_mutex_lock(&buffer_mutex);   // Lock the buffer mutex
    active_threads--;                    // Decrement active threads count
    pthread_mutex_unlock(&buffer_mutex); // Unlock the buffer mutex

    pthread_exit(NULL);
}

// Function to copy a file
void copy_file(file_info_t *file_info)
{
    char buffer[BUFSIZ];
    ssize_t bytes_read, bytes_written;

    // Read from source file and write to destination file
    while ((bytes_read = read(file_info->src_fd, buffer, BUFSIZ)) > 0)
    {
        bytes_written = write(file_info->dst_fd, buffer, bytes_read);
        if (bytes_written == -1)
        {
            perror("write");
            return;
        }
        total_bytes_copied += bytes_written;
    }

    if (bytes_read == -1)
    {
        perror("read");
    }

    // Update files copied count
    pthread_mutex_lock(&buffer_mutex);
    files_copied++;
    pthread_mutex_unlock(&buffer_mutex);

    printf("Copied: %s -> %s\n", file_info->src_path, file_info->dst_path);
}
