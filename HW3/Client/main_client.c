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


int write_command_to_server_fifo(int fd_client_cmd, char *command)
{

    ssize_t bytes_written_cmd = write(fd_client_cmd, command, strlen(command));
    if (bytes_written_cmd == -1)
    {
        perror("Failed to write command to client command FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if (bytes_written_cmd == 0)
    {
        perror("Failed to write command to client command FIFO");
        return 0;
    }

    return 1;
}
    


void cleanup()
{
    // Close file descriptors
    if (fd_client_cmd != -1)
    {
        close(fd_client_cmd);
    }
    if (fd_client_res != -1)
    {
        close(fd_client_res);
    }

    // Unlink fifos
    unlink(server_req_fifo);
    unlink(client_res_fifo);
}

void sigint_handler(int signum)
{
    // Write to the terminal without printf
    write(STDOUT_FILENO, "SIGINT received. Exiting...\n", 28);
    handle_quit_command(&fd_client_cmd, &fd_client_res, "quit", cleanup);
    exit(EXIT_SUCCESS);
}

void write_to_server_fifo(int server_pid, int child_pid)
{
    char server_fifo[SERVER_FIFO_LEN];
    sprintf(server_fifo, SERVER_FIFO, server_pid);
    //printf("Server FIFO: %s\n", server_fifo);

    // Write arguments and process id to SERVER_FIFO
    int fd_server_reg = open(server_fifo, O_WRONLY);
    if (fd_server_reg == -1)
    {
        perror("Failed to open SERVER_FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    struct client_info cli_info;
    cli_info.pid = child_pid;
    cli_info.wait = 0;
    getcwd(cli_info.cwd, CWD_SIZE);

    ssize_t bytes_written = write(fd_server_reg, &cli_info, sizeof(struct client_info));
    if (bytes_written == -1)
    {
        perror("Failed to write to fd_server_reg");
        cleanup();
        exit(EXIT_FAILURE);
    }

    close(fd_server_reg);
    //printf("Arguments and process id written to SERVER_FIFO.\n");
}

void open_named_pipes()
{
    sprintf(server_req_fifo, SERVER_REQ_FIFO, getpid());
    create_named_pipe(server_req_fifo);

    sprintf(client_res_fifo, CLIENT_RES_FIFO, getpid());
    create_named_pipe(client_res_fifo);

    // Open client response FIFO in read mode
    fd_client_res = open(client_res_fifo, O_RDONLY);
    if (fd_client_res == -1)
    {
        perror("Failed to open client response FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }
}

void handle_client_response()
{
    // Read 1 byte from parent
    char read_byte;
    ssize_t bytes_read = read(fd_client_res, &read_byte, sizeof(char));
    if (bytes_read == -1)
    {
        perror("Failed to read from parent");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Check if the read byte is integer "1"
    if (read_byte == '1')
    {
        printf("Connection is successful!\n");
    }
}

void handle_user_input()
{
    char command[256] = {0};
    char response[256] = {0};
    while (1)
    {
        printf("Enter command: ");
        fflush(stdout);

        // Read the command from user
        ssize_t bytes_read_cmd = read(STDIN_FILENO, command, sizeof(command) - 1);  // Leave space for null-terminator
        if (bytes_read_cmd <= 0) {
            if (bytes_read_cmd == 0) printf("No input provided, exiting.\n");
            else perror("Failed to read command from stdin");
            cleanup();
            exit(bytes_read_cmd == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
        }
        // TODO printf("command: %s\n", command);
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

        //words
        for(int i=0; i<num_words; i++)
        {
            printf("word[%d]: %s\n", i, words[i]);
        }

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
            handle_upload_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup, dir_syncs, server_req_fifo);
        }   

        else if(strcmp(command, "download")==0)
        {
            handle_download_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup);
        }

        else if(strcmp(command, "arch")==0)
        {
            handle_arch_command(&fd_client_cmd, &fd_client_res, words, num_words, cleanup, write_command_to_server_fifo);
        }
                else
        {
            printf("Invalid command!\n");
        }

        // CLean the command str
        memset(command, 0, sizeof(command));
        memset(response, 0, sizeof(response));
    }
}

int main(int argc, char *argv[])
{
    int shm_fd;
    void *shm_addr;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Connect/tryConnect> <ServerPID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connection...\n");

    // Get the current directory
    char dirname[256];
    getcwd(dirname, sizeof(dirname));

    dir_syncs = (struct dir_sync *)malloc(sizeof(struct dir_sync) * NUM_OF_DIR_FILE);

    if (initSafeDir(dirname, dir_syncs) == -1)
    {
        exit(EXIT_FAILURE);
    }

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

    const char *mode = argv[1];
    int server_pid = atoi(argv[2]);
    int child_pid = getpid();

    write_to_server_fifo(server_pid, child_pid);
    open_named_pipes();
    handle_client_response();

    fd_client_cmd = open(server_req_fifo, O_WRONLY);
    if (fd_client_cmd == -1)
    {
        printf("Failed to open client command FIFO\n");
        perror("Failed to open client command FIFO");
        cleanup();
        exit(EXIT_FAILURE);
    }
    handle_user_input();

    // Cleanup: Do not destroy semaphores here, just unmap and close
    munmap(shm_addr, SHM_SIZE);
    close(shm_fd);
    cleanup();

    return 0;
}
