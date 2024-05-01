#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

#include "../Util/client_info_queue.h"
#include "../Sync/synch.h"
#include "../Util/common.h"
#include <time.h>

/**
 * Cleans up child processes spawned by the server.
 * 
 * @param dir_syncs The directory synchronization data structure.
 */
void cleanup_child_processes(struct dir_sync *dir_syncs);

/**
 * Cleans up resources and performs necessary cleanup operations.
 * 
 * @param server_fd The file descriptor of the server.
 * @param client_res_fd The file descriptor of the client response.
 * @param clienet_req_fd The file descriptor of the client request.
 */
void cleanup(int server_fd, int client_res_fd, int clienet_req_fd, struct dir_sync *dir_syncs);

/**
 * Creates a shared memory segment and maps it to the process's address space.
 * 
 * @param shm_name The name of the shared memory segment.
 * @param shm_ptr A pointer to the shared memory segment.
 * @param shm_size The size of the shared memory segment.
 */
void create_shared_memory(const char *shm_name, void **shm_ptr, int shm_size);

/**
 * Writes a message to the log file. 
 * 
 * @param LOG_FILENAME The name of the log file.
 * @param message The message to be written to the log file.
 */
void write_log_file(char *message, struct dir_sync *dir_syncs);

/**
 * Writes a message to the log file with the current time.
 * 
 * @param LOG_FILENAME The name of the log file.
 * @param message The message to be written to the log file.
 */
void region_reader_logger(struct dir_sync *sync_dir, int pid, char *filename, int enter);


/**
 * Writes a message to the log file with the current time.
 * 
 * @param LOG_FILENAME The name of the log file.
 * @param message The message to be written to the log file.
 * @param enter 1 if entering the region, 0 if exiting the region
 */
void region_writer_logger(struct dir_sync *sync_dir, int pid, char *filename, int enter);


#endif