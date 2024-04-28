#include "../Util/common.h"
#include "../Util/client_info_queue.h"
#include "../Sync/synch.h"
#include "client_list.h"
#include "server_util.h"
#include "command_operation.h"

#include <dirent.h>

// Function prototypes
void handle_signal(int sig);
void setup_signal_handler();
void server_loop(char *dirname);


// Global variables
char log_filename[256];
struct dir_sync *dir_syncs = NULL;
struct client_list_wrapper *list_clients = NULL;
struct queue client_queue;
int global_ppid;

int main(int argc, char *argv[])
{

    // Check if the correct number of command line arguments are provided
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <dirname> <max.#ofClients>\n", argv[0]);
        exit(1);
    }

    // Extract the directory name and maximum number of clients from command line arguments
    char *dirname = argv[1]; // This line extracts the directory name from the command line argument and assigns it to the dirname variable.
    int max_clients = atoi(argv[2]); // This line converts the second command line argument to an integer and assigns it to the max_clients variable. It represents the maximum number of clients.
    //int server_pid = getpid(); // This line retrieves the process ID (PID) of the server process and assigns it to the server_pid variable.
    //int shm_fd; // This line declares an integer variable shm_fd which will be used to store the file descriptor of the shared memory segment.
    //int shm_fd2; // This line declares an integer variable shm_fd2 which will be used to store the file descriptor of the shared memory segment.
    global_ppid = getpid();
    // Print server start message
    printf("Server Started PID %d...\nWaiting for clients...\n", getpid());


    // Create and initialize shared memory for directory synchronization
    // create_shared_memory(SHM_NAME, (void **)&dir_syncs, sizeof(struct file_sync) * NUM_OF_DIR_FILE + sizeof(Semaphores) + sizeof(int) * 2);

    int shm_fd;
    int shm_size = sizeof(struct file_sync) * NUM_OF_DIR_FILE + sizeof(Semaphores) + sizeof(int) * 2;
    /* Create a shared memory segment */
    if ((shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("Error creating shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Set the size of the shared memory segment */
    if (ftruncate(shm_fd, shm_size) == -1)
    {
        perror("Error setting size of shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Map the shared memory segment to the process's address space */
    if ((dir_syncs = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
    {
        perror("Error mapping shared memory segment");
        exit(EXIT_FAILURE);
    }

    // Create and initialize shared memory for client list
    // create_shared_memory(SHM_NAME_CLIENTS, (void **)&list_clients, sizeof(int) + sizeof(int) * 10);

    int shm_fd2;
    int shm_size2 = sizeof(int) + sizeof(int) * 10;
    /* Create a shared memory segment */
    if ((shm_fd2 = shm_open(SHM_NAME_CLIENTS, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("Error creating shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Set the size of the shared memory segment */
    if (ftruncate(shm_fd2, shm_size2) == -1)
    {
        perror("Error setting size of shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Map the shared memory segment to the process's address space */
    if ((list_clients = mmap(NULL, shm_size2, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0)) == MAP_FAILED)
    {
        perror("Error mapping shared memory segment");
        exit(EXIT_FAILURE);
    }

    // Initialize the safe directory
    if (initSafeDir(dirname, dir_syncs, max_clients) == -1)
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the client list
    initialize_client_list(list_clients);

    // Setup signal handler for SIGINT
    setup_signal_handler();

    // Start the server loop
    server_loop(dirname);

    return 0;
}


/**
 * Signal handler function to handle SIGINT and SIGCHLD signals.
 * 
 * @param sig The signal number.
 */
void handle_signal(int sig)
{
    if (sig == SIGINT)
    {
        // Print a message indicating the received SIGINT signal
        const char *msg = "Server terminating...\n";
        write(STDOUT_FILENO, msg, strlen(msg));

        // Remove the server FIFO and close the safe directory
        char server_fifo[256];
        sprintf(server_fifo, SERVER_FIFO, global_ppid);
        unlink(server_fifo);
        shm_unlink(SHM_NAME);
        shm_unlink(SHM_NAME_CLIENTS);
        closeSafeDir(dir_syncs);

        // Traverse and send SIGINT signal to all child pids in the client list
        for (int i = 0; i < list_clients->counter; i++)
        {
            // printf("Killing client %d\n", list_clients->clients[i]);
            kill(list_clients->clients[i], SIGINT);
        }

        // Exit the server process
        exit(0);
    }
    else if (sig == SIGCHLD)
    {
        // Cleanup child processes
        cleanup_child_processes(dir_syncs);
    }
}

/**
 * Sets up the signal handler for SIGINT and SIGCHLD signals.
 */
void setup_signal_handler()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = SA_RESTART; // Restart sys-calls automatically
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

/**
 * The main server loop that handles client connections and commands.
 * 
 * @param dirname The name of the directory to be created if it doesn't exist.
 * @param max_clients The maximum number of clients that can be connected simultaneously.
 */
void server_loop(char *dirname)
{
    char server_fifo[256];
    int server_pid = getpid();
    sprintf(server_fifo, SERVER_FIFO, server_pid);
    DIR *dir = opendir(dirname);
    // If dirname is not existed, create it
    if (dir == NULL)
    {
        if (mkdir(dirname, 0777) == -1)
        {
            perror("Failed to create directory");
            exit(EXIT_FAILURE);
        }
    }
    closedir(dir);

    create_named_pipe(server_fifo); // Create the server FIFO

    int server_fd = open(server_fifo, O_RDONLY);
    if (server_fd == -1)
    {
        perror("Failed to open server FIFO");
        cleanup(server_fd, -1, -1, dir_syncs);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Read client info from the server FIFO
        struct client_info cli_info; // Client info structure
        ssize_t num_read = read(server_fd, &cli_info, sizeof(cli_info));
        if (num_read == -1)
        {
            perror("Failed to read from server FIFO");
            cleanup(server_fd, -1, -1, dir_syncs);
            exit(EXIT_FAILURE);
        }
        else if (num_read == 0 && is_queue_empty(&client_queue))
        {
            // If no data is read and the client queue is empty, continue to the next iteration
            continue;
        }
        else if (num_read != 0)
        {
            // Enqueue the client info into the client queue and add the client to the list of clients
            enqueue(&client_queue, cli_info);
            add_client(list_clients, cli_info.pid);
        }
        
        // Check if the client queue is full
        if (sem_trywait(&dir_syncs->sems.empty) != 0)
        {
            printf("Connection request %d is refused. Queue is full. \n", cli_info.pid);
            if(cli_info.mode==1)
            {
                dequeue(&client_queue);
                char client_res_fifo[256]; // Client response FIFO
                sprintf(client_res_fifo, CLIENT_RES_FIFO, cli_info.pid); // Create the client response FIFO
                int client_res_fd = open(client_res_fifo, O_WRONLY); // Open the client response FIFO

                if (client_res_fd == -1)
                {
                    perror("Failed to open client response FIFO");
                    cleanup(server_fd, client_res_fd, -1, dir_syncs);
                    exit(EXIT_FAILURE);
                }

                char response = '0'; // This will mean that connection is successful
                ssize_t num_written = write(client_res_fd, &response, sizeof(response)); // Write the response to the client response FIFO
                if (num_written == -1)
                {
                    perror("Failed to write to client response FIFO");
                    cleanup(server_fd, client_res_fd, -1, dir_syncs);
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            sem_wait(&dir_syncs->sems.empty);
        }

        cli_info = dequeue(&client_queue);

        pid_t child_pid = fork();
        if (child_pid == -1)
        {
            perror("Failed to fork child process");
            cleanup(server_fd, -1, -1, dir_syncs);
            exit(EXIT_FAILURE);
        }

        if (child_pid == 0)
        {
            printf("Client %d is connected!\n", cli_info.pid);
            char client_res_fifo[256]; // Client response FIFO
            sprintf(client_res_fifo, CLIENT_RES_FIFO, cli_info.pid); // Create the client response FIFO
            int client_res_fd = open(client_res_fifo, O_WRONLY); // Open the client response FIFO

            if (client_res_fd == -1)
            {
                perror("Failed to open client response FIFO");
                cleanup(server_fd, client_res_fd, -1, dir_syncs);
                exit(EXIT_FAILURE);
            }

            char response = '1'; // This will mean that connection is successful
            ssize_t num_written = write(client_res_fd, &response, sizeof(response)); // Write the response to the client response FIFO
            if (num_written == -1)
            {
                perror("Failed to write to client response FIFO");
                cleanup(server_fd, client_res_fd, -1, dir_syncs);
                exit(EXIT_FAILURE);
            }

            char server_req_fifo[256]; // Server request FIFO
            sprintf(server_req_fifo, SERVER_REQ_FIFO, cli_info.pid); // Create the server request FIFO
            int server_req_fd = open(server_req_fifo, O_RDONLY); // Open the server request FIFO
            if (server_req_fd == -1)
            {
                perror("Failed to open server request FIFO");
                cleanup(server_fd, client_res_fd, server_req_fd, dir_syncs);
                exit(EXIT_FAILURE);
            }

            char command[256] = {0}; // Command buffer that will store the command received from the client
            while(1)
            {
                ssize_t num_read = read(server_req_fd, command, sizeof(command)); // Read the command from the server request FIFO
                if (num_read == -1)
                {
                    perror("Failed to read from server request FIFO");
                    cleanup(server_fd, client_res_fd, server_req_fd, dir_syncs);
                    exit(EXIT_FAILURE);
                }

                if (num_read == 0)
                {
                    memset(command, 0, sizeof(command));
                    continue;
                }

                // Handle different commands
                if (strcmp(command, "list") == 0)
                {
                    handle_list_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
                }
                else if(strcmp(command, "quit") == 0)
                {       
                    handle_quit_command(client_res_fd, server_fd, server_req_fd, cli_info, list_clients, dir_syncs);
                }
                else if(strcmp(command, "killServer") == 0)
                {
                    handle_killServer_command(client_res_fd, server_fd, server_req_fd, dir_syncs, list_clients);
                }
                else if(strncmp(command, "readF", 5)==0)
                {
                    handle_readF_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
                }
                else if(strncmp(command, "writeF", 6)==0)
                {
                    handle_writeF_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs);
                }
                else if(strncmp(command, "download", 8)==0)
                {
                    handle_download_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
                }
                else if(strncmp(command, "upload", 6)==0)
                {
                    handle_upload_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs);
                }
                else if(strncmp(command, "archServer", 10)==0)
                {
                    handle_archive_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs);
                }
                else if(strncmp(command, "help", 4)==0)
                {
                    handle_help_command(client_res_fd, server_fd, server_req_fd, dir_syncs);
                }
                else
                {
                    printf("command: %s\n", command);
                    printf("Invalid command\n");
                }

                // Clear the command buffer
                memset(command, 0, sizeof(command));
            }
        }

        else
            continue;
        
    }

    cleanup(server_fd, -1, -1, dir_syncs);
}
