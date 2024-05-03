/**
 * @file client_command_operations.h
 * @brief Header file containing function declarations for handling client commands.
 *
 * This header file provides the function declarations for handling various client commands,
 * such as list, quit, killServer, help, readF, writeF, upload, download, and arch.
 * These functions are responsible for performing the necessary operations based on the command
 * received from the client.
 *
 * The functions in this header file are used in conjunction with other modules and libraries
 * to implement the functionality of a client in a system programming project.
 */

#ifndef CLIENT_COMMAND_OPERATIONS_H
#define CLIENT_COMMAND_OPERATIONS_H

#include "../Sync/synch.h"
#include "../Util/client_info_queue.h"
#include "../Util/common.h"
#include "../Sync/synch.h"

/**
 * @brief Handles the list command received from the client.
 *
 * This function handles the list command received from the client and performs the necessary
 * operations to retrieve and display the list of files available on the server.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param words Array of strings containing the command and its arguments.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 */
void handle_list_command(int *fd_client_cmd, int *fd_client_res, char** words, void (*cleanup)());

/**
 * @brief Handles the quit command received from the client.
 *
 * This function handles the quit command received from the client and performs the necessary
 * operations to terminate the client application.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param command The quit command string.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 */
void handle_quit_command(int *fd_client_cmd, int *fd_client_res, char* command, void (*cleanup)());

/**
 * @brief Handles the killServer command received from the client.
 *
 * This function handles the killServer command received from the client and performs the necessary
 * operations to terminate the server application.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param command The killServer command string.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 */
void handle_killServer_command(int *fd_client_cmd, int *fd_client_res, char* command, void (*cleanup)());

/**
 * @brief Handles the help command received from the client.
 *
 * This function handles the help command received from the client and displays the available
 * commands and their usage.
 */
void handle_help_command(char** words, int num_words);

/**
 * @brief Handles the readF command received from the client.
 *
 * This function handles the readF command received from the client and performs the necessary
 * operations to read the contents of a file on the server and send it to the client.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param words Array of strings containing the command and its arguments.
 * @param num_words Number of words in the words array.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 */
void handle_readF_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)());

/**
 * @brief Handles the writeF command received from the client.
 *
 * This function handles the writeF command received from the client and performs the necessary
 * operations to write the contents of a file received from the client to the server.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param words Array of strings containing the command and its arguments.
 * @param num_words Number of words in the words array.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 */
void handle_writeF_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)());

/**
 * @brief Handles the upload command received from the client.
 *
 * This function handles the upload command received from the client and performs the necessary
 * operations to upload a file from the client to the server.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param words Array of strings containing the command and its arguments.
 * @param num_words Number of words in the words array.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 * @param dir_syncs Pointer to the directory synchronization data structure.
 * @param server_req_fifo The name of the server request FIFO.
 */
void handle_upload_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)(), const char* server_req_fifo);

/**
 * @brief Handles the download command received from the client.
 *
 * This function handles the download command received from the client and performs the necessary
 * operations to download a file from the server to the client.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param words Array of strings containing the command and its arguments.
 * @param num_words Number of words in the words array.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 */
void handle_download_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)());

/**
 * @brief Handles the arch command received from the client.
 *
 * This function handles the arch command received from the client and performs the necessary
 * operations to archive a directory on the server.
 *
 * @param fd_client_cmd Pointer to the file descriptor for the client command FIFO.
 * @param fd_client_res Pointer to the file descriptor for the client response FIFO.
 * @param words Array of strings containing the command and its arguments.
 * @param num_words Number of words in the words array.
 * @param cleanup Pointer to the cleanup function to be called after handling the command.
 * @param write_command_to_server_fifo Pointer to the function for writing a command to the server FIFO.
 */
void handle_arch_command(int *fd_client_cmd, int *fd_client_res, char** words, int num_words, void (*cleanup)(), int (*write_command_to_server_fifo)(int, char*));

#endif