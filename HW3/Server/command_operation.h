#ifndef COMMAND_OPERATION_H
#define COMMAND_OPERATION_H

#include "../Util/common.h"
#include "../Util/client_info_queue.h"

/**
 * Handles the "list" command.
 * 
 * @param command The command string.
 * @param dirname The name of the directory to list.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 */
void handle_list_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo);

/**
 * Handles the "readF" command.
 * 
 * @param command The command string.
 * @param dirname The name of the directory to read from.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 * @param num_read The number of lines to read from the file.
 */
void handle_readF_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read);

/**
 * Handles reading a specific line from a file.
 * 
 * @param file The file to read from.
 * @param readF The readF_command structure.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 */
void handle_specific_line(FILE *file, readF_command readF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo);

/**
 * Handles reading the whole file.
 * 
 * @param file The file to read from.
 * @param readF The readF_command structure.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 */
void handle_whole_file(FILE *file, readF_command readF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo);

/**
 * Handles the "quit" command.
 * 
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param cli_info The client_info structure.
 */
void handle_quit_command(int client_res_fd, int server_fd, int server_req_fd, struct client_info cli_info);

/**
 * Handles the "killServer" command.
 * 
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param server_pid The server PID.
 */
void handle_killServer_command(int client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, int server_pid);

/**
 * Handles the "writeF" command.
 * 
 * @param command The command string.
 * @param dirname The name of the directory to write to.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 * @param num_read The number of lines to read from the client.
 */
void handle_writeF_command(char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read);

/**
 * Handles writing a specific line to a file.
 * 
 * @param dirname The name of the directory to write to.
 * @param writeF The writeF_command structure.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 */
void handle_specific_line_write(char *dirname, writeF_command writeF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo);

/**
 * Handles writing the whole file.
 * 
 * @param dirname The name of the directory to write to.
 * @param writeF The writeF_command structure.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 */
void handle_whole_file_write(char *dirname, writeF_command writeF, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo);


/**
 * Handles downloading a file to client side.
 * @param command The command string.
 * @param dirname The name of the directory to download from.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 * @param num_read The number of lines to read from the client.
 * */
void handle_download_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read);

/**
 * Handles uploading a file to server side.
 * @param command The command string.
 * @param dirname The name of the directory to upload to.
 * @param client_res_fd The file descriptor for the client response FIFO.
 * @param server_fd The file descriptor for the server FIFO.
 * @param server_req_fd The file descriptor for the server request FIFO.
 * @param dir_syncs An array of dir_sync structures.
 * @param client_res_fifo The name of the client response FIFO.
 * @param num_read The number of lines to read from the client.
*/
void handle_upload_command(const char* command, const char* dirname, int *client_res_fd, int server_fd, int server_req_fd, struct dir_sync *dir_syncs, char* client_res_fifo, int num_read);


#endif // COMMAND_OPERATION_H