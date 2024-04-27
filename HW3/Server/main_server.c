#include "../Util/common.h"
#include "../Util/client_info_queue.h"
#include "../Sync/synch.h"
#include "server_util.h"
#include "command_operation.h"
#include <dirent.h>

// Function prototypes
void handle_signal(int sig);
void setup_signal_handler();
void server_loop(const char *dirname, int max_clients);
void cleanup(int server_fd);

// Global variables
char log_filename[256];
struct dir_sync *dir_syncs;
struct queue client_queue;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <dirname> <max.#ofClients>\n", argv[0]);
        exit(1);
    }

    char *dirname = argv[1];
    int max_clients = atoi(argv[2]);
    int server_pid = getpid();

    //create_named_pipe(SERVER_FIFO);

    printf("Server Started PID %d...\nWaiting for clients...\n", getpid());

    int shm_fd;
    /* Create a shared memory segment */
    if ((shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("Error creating shared memory segment");
        exit(EXIT_FAILURE);
    }

    /* Set the size of the shared memory segment */
    int shm_size = sizeof(int) * 2 + sizeof(struct dir_sync) * NUM_OF_DIR_FILE;

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

    if (initSafeDir(dirname, dir_syncs) == -1)
    {
        exit(EXIT_FAILURE);
    }

    setup_signal_handler();

    server_loop(dirname, max_clients);

    return 0;
}

void cleanup(int server_fd)
{
    closeSafeDir(dir_syncs);
    char server_fifo[256];
    sprintf(server_fifo, SERVER_FIFO, getpid());
    unlink(server_fifo);

    if (server_fd != -1)
    {
        close(server_fd);
    }
}

void handle_signal(int sig)
{
    if (sig == SIGINT)
    {
        const char *msg = "Received SIGINT. Server terminating...\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        char server_fifo[256];
        sprintf(server_fifo, SERVER_FIFO, getpid());
        closeSafeDir(dir_syncs);

        // Cloes and unlink server_fifo
        unlink(server_fifo);
        exit(0);
    }
    else if (sig == SIGCHLD)
    {
        cleanup_child_processes(dir_syncs);
    }
}

void setup_signal_handler()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = SA_RESTART; // Restart sys-calls automatically
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

void server_loop(const char *dirname, int max_clients)
{
    char server_fifo[256];
    int server_pid = getpid();
    sprintf(server_fifo, SERVER_FIFO, server_pid);

    //mkdir(dirname, 0755);

    create_named_pipe(server_fifo);

    int server_fd = open(server_fifo, O_RDONLY);
    if (server_fd == -1)
    {
        perror("Failed to open server FIFO");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        struct client_info cli_info;
        ssize_t num_read = read(server_fd, &cli_info, sizeof(cli_info));
        if (num_read == -1)
        {
            perror("Failed to read from server FIFO");
            cleanup(server_fd);
            exit(EXIT_FAILURE);
        }
        else if (num_read == 0 && is_queue_empty(&client_queue))
        {
            continue;
        }
        else if (num_read != 0)
        {
            // printf("Received data: PID=%d, Data=%s, bytes:%zd\n", cli_info.pid, cli_info.cwd, num_read);
            enqueue(&client_queue, cli_info);
            print_queue(&client_queue);

            // printf("Client info enqueued\n");
            // printf("Client PID: %d\n", cli_info.pid);
            // printf("Client CWD: %s\n", cli_info.cwd);
            // printf("Client wait: %d\n", cli_info.wait);
        }

        if (sem_trywait(&dir_syncs->sems.empty) != 0)
        {
            if (errno == EAGAIN)
            {
                printf("Connection request %d is refused. Queue is full. \n", cli_info.pid);
                sem_wait(&dir_syncs->sems.empty);
            }
            else
            {
                printf("Connection request %d is refused. Queue is full. \n", cli_info.pid);
                sem_wait(&dir_syncs->sems.empty);
            }
        }

        // Print all queue
        print_queue(&client_queue);

        cli_info = dequeue(&client_queue);

        pid_t child_pid = fork();
        if (child_pid == -1)
        {
            perror("Failed to fork child process");
            cleanup(server_fd);
            exit(EXIT_FAILURE);
        }

        if (child_pid == 0)
        {
            printf("Client %d is connected!\n", cli_info.pid);
            char client_res_fifo[256];
            sprintf(client_res_fifo, CLIENT_RES_FIFO, cli_info.pid);
            int client_res_fd = open(client_res_fifo, O_WRONLY);
            // printf("child pid: %d and fifo: %s\n", cli_info.pid, client_res_fifo);

            if (client_res_fd == -1)
            {
                perror("Failed to open client response FIFO");
                close(server_fd);
                close(client_res_fd);
                exit(EXIT_FAILURE);
            }

            char response = '1';
            ssize_t num_written = write(client_res_fd, &response, sizeof(response));
            if (num_written == -1)
            {
                perror("Failed to write to client response FIFO");
                close(server_fd);
                close(client_res_fd);
                exit(EXIT_FAILURE);
            }


            char server_req_fifo[256];
            sprintf(server_req_fifo, SERVER_REQ_FIFO, cli_info.pid);
            int server_req_fd = open(server_req_fifo, O_RDONLY);
            if (server_req_fd == -1)
            {
                perror("Failed to open server request FIFO");
                close(server_fd);
                close(client_res_fd);
                exit(EXIT_FAILURE);
            }

            char command[256] = {0};
            // read the command from the client
            while(1)
            {
               
                printf("test");
                ssize_t num_read = read(server_req_fd, command, sizeof(command));
                printf("test, ommand %s and %d\n", command, num_read);
                
                // List the folders in the current dir and write them to the pipe client_res_fd

                if (strcmp(command, "list") == 0)
                {
                    handle_list_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo);
                }

                else if(strcmp(command, "quit") == 0)
                {       
                    handle_quit_command(client_res_fd, server_fd, server_req_fd, cli_info);
                }

                else if(strcmp(command, "killServer") == 0)
                {
                    handle_killServer_command(client_res_fd, server_fd, server_req_fd, dir_syncs, server_pid);
                }

                else if(strncmp(command, "readF", 5)==0)
                {
                    handle_readF_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo, num_read);
                }

                else if(strncmp(command, "writeF", 6)==0)
                {
                    handle_writeF_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo, num_read);
                }

                else if(strncmp(command, "download", 8)==0)
                {
                    handle_download_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo, num_read);
                }

                else if(strncmp(command, "upload", 6)==0)
                {
                    handle_upload_command(command, dirname, &client_res_fd, server_fd, server_req_fd, dir_syncs, client_res_fifo, num_read);
                }

                else
                {
                    printf("Invalid command\n");
                }
                memset(command, 0, sizeof(command));

            }
        }
    
        else
        {
            // printf("Parent loop continue...\n");
            continue;
        }
    }
    

    closeSafeDir(dir_syncs);
    close(server_fd);
    unlink(server_fifo);
}
