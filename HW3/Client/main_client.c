#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "../Util/common.h"
#include "../Util/client_info_queue.h"
#include "client_command_operations.h"

char server_req_fifo[256];
char client_res_fifo[256];
int fd_client_cmd = -1;
int fd_client_res = -1;
struct dir_sync *dir_syncs;

/**
 * Cleans up resources used by the client.
 * Closes file descriptors and unlinks named pipes.
 */
void cleanup()
{
    // Close file descriptors
    if (fd_client_cmd != -1)
    {
        close(fd_client_cmd); // Close the client command FIFO file descriptor
    }
    if (fd_client_res != -1)
    {
        close(fd_client_res); // Close the client response FIFO file descriptor
    }

    // Unlink named pipes
    if (unlink(server_req_fifo) == -1)
    {
        perror("Failed to unlink server request FIFO");
    }
    if (unlink(client_res_fifo) == -1)
    {
        perror("Failed to unlink client response FIFO");
    }
}


/**
 * Writes a command to the client command FIFO.
 * 
 * @param fd_client_cmd The file descriptor of the client command FIFO.
 * @param command The command to be written.
 * @return 1 if the command is successfully written, 0 otherwise.
 */
int write_command_to_server_fifo(int fd_client_cmd, char *command)
{
    // Write the command to the client command FIFO
    ssize_t bytes_written_cmd = write(fd_client_cmd, command, strlen(command));
    if (bytes_written_cmd == -1)
    {
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if (bytes_written_cmd == 0)
    {
        return 0;
    }

    return 1;
}


/**
 * Signal handler for SIGINT.
 * 
 * @param signum The signal number.
 */
void sigint_handler()
{
    // Write a message to the terminal without using printf
    write(STDOUT_FILENO, "SIGINT received. Exiting...\n", 28);
    
    // Handle the "quit" command to clean up resources and exit gracefully
    handle_quit_command(&fd_client_cmd, &fd_client_res, "quit", cleanup);

    // Exit the program successfully
    exit(EXIT_SUCCESS);
}

/**
 * Writes the client information to the server FIFO.
 * 
 * @param server_pid The process ID of the server.
 * @param child_pid The process ID of the child.
 */
void write_to_server_fifo(int server_pid, int child_pid, int mode)
{
    char server_fifo[SERVER_FIFO_LEN];
    sprintf(server_fifo, SERVER_FIFO, server_pid);

    // Open the server FIFO in write-only mode
    int fd_server_reg = open(server_fifo, O_WRONLY);
    if (fd_server_reg == -1)
    {
        perror("Failed to open SERVER_FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }

    struct client_info cli_info;
    memset(&cli_info, 0, sizeof(cli_info));  // Zero out the entire structure

    
    cli_info.pid = child_pid;
    cli_info.mode = mode;
    getcwd(cli_info.cwd, CWD_SIZE);

    // Write the client information to the server FIFO
    ssize_t bytes_written = write(fd_server_reg, &cli_info, sizeof(struct client_info));
    if (bytes_written == -1)
    {
        perror("Failed to write to fd_server_reg");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Close the server FIFO
    close(fd_server_reg);
}

/**
 * Opens the named pipes for communication with the server.
 * 
 * This function creates and opens the server request FIFO and client response FIFO.
 * It also sets the file descriptor for the client response FIFO.
 */
void open_named_pipes()
{
    // Create and open the server request FIFO
    sprintf(server_req_fifo, SERVER_REQ_FIFO, getpid());
    create_named_pipe(server_req_fifo);

    // Create and open the client response FIFO
    sprintf(client_res_fifo, CLIENT_RES_FIFO, getpid());
    create_named_pipe(client_res_fifo);

    // Open the client response FIFO in read-only mode
    fd_client_res = open(client_res_fifo, O_RDONLY);
    if (fd_client_res == -1)
    {
        perror("Failed to open client response FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }
}
/**
 * Handles the response received from the client.
 * 
 * This function reads 1 byte from the client response FIFO and checks if the read byte is '1'.
 * If the read byte is '1', it prints "Connection is successful!".
 * Otherwise, it exits the program with failure.
 */
void handle_client_response()
{
    // Read 1 byte from the client response FIFO
    char read_byte;
    ssize_t bytes_read = read(fd_client_res, &read_byte, sizeof(char));
    if (bytes_read == -1)
    {
        perror("Failed to read from client response FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if (bytes_read == 0)
    {
        perror("Failed to read from client response FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Check if the read byte is '1'
    if (read_byte == '1')
    {
        printf("Connection is successful!\n");
    }
    else
    {
        printf("Connection failed!\n");
        cleanup();
        exit(EXIT_FAILURE);
    }
}


/**
 * Handles user input commands.
 * 
 * This function reads commands entered by the user from the standard input and processes them accordingly.
 * It supports commands such as "list", "quit", "killServer", "help", "readF", "writeF", "upload", "download", and "archServer".
 * 
 * @param fd_client_cmd The file descriptor for the client command FIFO.
 * @param fd_client_res The file descriptor for the client response FIFO.
 * @param words An array of strings containing the parsed command words.
 * @param num_words The number of words in the command.
 * @param cleanup A function pointer to the cleanup function.
 * @param dir_syncs An array of directory synchronization structures.
 * @param server_req_fifo The server request FIFO.
 */
void handle_user_input()
{
    char command[256] = {0}; // Buffer to store the command entered by the user
    char response[256] = {0}; // Buffer to store the response from the server

    while (1)
    {
        printf("Enter command: ");
        fflush(stdout); // Flush the output buffer to print the prompt

        // Read the command from user
        ssize_t bytes_read_cmd = read(STDIN_FILENO, command, sizeof(command) - 1);  // Leave space for null-terminator
        if (bytes_read_cmd <= 0) {
            if (bytes_read_cmd == 0)
            {
                // Clean the command string
                memset(command, 0, sizeof(command));
                memset(response, 0, sizeof(response));
                continue;
            }
            else 
            {
                perror("Failed to read command from stdin");
                cleanup();
                exit(bytes_read_cmd == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
            }
        }
        fflush(STDIN_FILENO);

        if (bytes_read_cmd == -1)
        {
            printf("Failed to read command from stdin\n");
            perror("Failed to read command from stdin");
            cleanup();
            exit(EXIT_FAILURE);
        }
        // Remove newline character from the command
        command[strcspn(command, "\n")] = '\0';

        // Parse the command into separate words
        char *words[4];
        int num_words = 0;
        char *token = strtok(command, " ");
        while (token != NULL && num_words < 4) {
            words[num_words] = token;
            num_words++;
            token = strtok(NULL, " ");
        }

        // Handle different commands based on the first word
        if(strcmp(words[0], "list")==0)
        {
            handle_list_command(&fd_client_cmd, &fd_client_res, words, cleanup);
        }
        else if(strcmp(command, "quit")==0)
        {
            handle_quit_command(&fd_client_cmd, &fd_client_res, command, cleanup);
        }
        else if(strcmp(command, "killServer")==0)
        {
            handle_killServer_command(&fd_client_cmd, &fd_client_res, command, cleanup);
        }
        else if(strcmp(command, "help")==0)
        {
            handle_help_command();
        }
        else if(strcmp(command, "readF")==0)
        {
            handle_readF_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup);
        }
        else if(strcmp(command, "writeF")==0)
        {
            handle_writeF_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup);
        }
        else if(strcmp(command, "upload")==0)
        {
            handle_upload_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup, server_req_fifo);
        }   
        else if(strcmp(command, "download")==0)
        {
            handle_download_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup);
        }
        else if(strcmp(command, "archServer")==0)
        {
            handle_arch_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup, write_command_to_server_fifo);
        }
        else
        {
            printf("Invalid command!\n");
        }

        // Clean the buffers
        memset(command, 0, sizeof(command));
        memset(response, 0, sizeof(response));
    }
}


int main(int argc, char *argv[])
{
    // Check the number of command-line arguments
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Connect/tryConnect> <ServerPID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connection...\n");

    // Get the current directory
    char dirname[256];
    getcwd(dirname, sizeof(dirname));

    // Set up signal handler for SIGINT (Ctrl+C) using sigaction
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Failed to set up signal handler for SIGINT");
        exit(EXIT_FAILURE);
    }

    // Get the mode and server PID from command-line arguments
    const char *mode_str = argv[1];
    int server_pid = atoi(argv[2]);
    int child_pid = getpid();

    int mode = 0;
    if(strcmp(mode_str, "connect")==0)
    {
        mode = 0;
    }
    else if(strcmp(mode_str, "tryConnect")==0)
    {
        mode = 1;
    }
    else
    {
        printf("Invalid mode\n");
        exit(EXIT_FAILURE);
    }

    // Write child PID to server FIFO
    write_to_server_fifo(server_pid, child_pid, mode);

    // Open named pipes for communication
    open_named_pipes();

    // Handle client response
    handle_client_response();

    // Open client command FIFO for writing
    fd_client_cmd = open(server_req_fifo, O_WRONLY);
    if (fd_client_cmd == -1)
    {
        printf("Failed to open client command FIFO\n");
        perror("Failed to open client command FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Handle user input
    handle_user_input();

    // Cleanup: Close shared memory file descriptor and perform cleanup operations
    cleanup();

    return 0;
}
