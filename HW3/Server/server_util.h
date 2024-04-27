#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

#include "../Util/client_info_queue.h"
#include "../Sync/synch.h"
#include "../Util/common.h"

/**
 * Cleans up child processes spawned by the server.
 * 
 * @param dir_syncs The directory synchronization data structure.
 */
void cleanup_child_processes(struct dir_sync *dir_syncs);

/**
 * Cleans up server resources and closes file descriptors.
 * 
 * @param server_fd The server file descriptor.
 * @param client_res_fd The client response file descriptor.
 * @param server_req_fd The server request file descriptor.
 * @param dir_syncs The directory synchronization data structure.
 * @param server_fifo_path The path to the server FIFO.
 */
void cleanup_server(int server_fd, int client_res_fd, int server_req_fd, struct dir_sync *dir_syncs, int parent_pid);

#endif