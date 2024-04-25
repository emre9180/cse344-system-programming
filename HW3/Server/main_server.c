#include "../Util/common.h"
#include "../Util/client_info_queue.h"
#include <dirent.h>
#include "../Sync/synch.h"

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
                printf("test, ommand %s\n", command);
                
                // List the folders in the current dir and write them to the pipe client_res_fd

                if (strcmp(command, "list") == 0)
                {printf("test");
                    enterRegionReader(getSafeFile(dir_syncs, "05319346629"));
                    printf("lst command\n");
                    ftruncate(client_res_fd, 0);
                    // printf("dirname: %s\n", dirname);
                    DIR *dir;
                    struct dirent *ent;

                    if ((dir = opendir(dirname)) != NULL) 
                    {
                        int hasEntry = 0; // Flag to check if there is any file or folder
                        while ((ent = readdir(dir)) != NULL) {
                            if (ent->d_type == DT_DIR || ent->d_type == DT_REG) {
                                char *name = ent->d_name;
                                // Exclude hidden files and directories
                                if (name[0] != '.') {
                                    printf("Entry name: %s\n", name);
                                    write(client_res_fd, name, strlen(name));
                                    write(client_res_fd, "\n", 1);
                                    hasEntry = 1;
                                }
                            }
                        }

                        closedir(dir);

                        // If there is no file or folder, clear the FIFO
                        if (!hasEntry) {
                            const char* error_message = "Nothing has been found.";
                            if (write(client_res_fd, error_message, strlen(error_message)) == -1)
                            {
                                perror("Failed to write error message to client response FIFO");
                                close(server_fd);
                                close(client_res_fd);
                                close(server_req_fd);
                                exitRegionReader(getSafeFile(dir_syncs, "05319346629"));
                                exit(EXIT_FAILURE);
                            }
                        }
                    } 

                    else
                    {
                        perror("Failed to open directory:");
                        close(server_fd);
                        close(client_res_fd);
                        close(server_req_fd);
                        exitRegionReader(getSafeFile(dir_syncs, "05319346629"));
                        exit(EXIT_FAILURE);
                    }
                    
                    exitRegionReader(getSafeFile(dir_syncs, "05319346629"));
                }


                else if(strcmp(command, "quit") == 0)
                {
                    int response = 1;
                    ssize_t num_written = write(client_res_fd, &response, sizeof(response));
                    if (num_written == -1)
                    {
                        perror("Failed to write to client response FIFO");
                        close(server_fd);
                        close(client_res_fd);
                        exit(EXIT_FAILURE);
                    }
                    close(server_fd);
                    close(client_res_fd);
                    close(server_req_fd);
                    printf("Client %d is disconnected!\n", cli_info.pid);
                    exit(EXIT_SUCCESS);
                }

                else if(strcmp(command, "killServer") == 0)
                {
                    int response = 1;
                    ssize_t num_written = write(client_res_fd, &response, sizeof(int));
                    if (num_written == -1)
                    {
                        perror("Failed to write to client response FIFO");
                        close(server_fd);
                        close(client_res_fd);
                        exit(EXIT_FAILURE);
                    }
                    close(server_fd);
                    close(client_res_fd);
                    close(server_req_fd);
                    printf("Server is killed!\n");
                    // Send SIGTERM to the entire process group
                    if (kill(-getpgid(0), SIGTERM) == -1) {
                        perror("Failed to send SIGTERM to the process group");
                        exit(EXIT_FAILURE);
                    }
                }

                else if(strncmp(command, "readF", 5)==0)
                {
                    /* */
                    // Parse the command into separate words
                    char *words[4];
                    int num_words = 0;
                    char *token = strtok(command, " ");
                    while (token != NULL && num_words < 4) {
                        words[num_words] = token;
                        num_words++;
                        token = strtok(NULL, " ");
                    }

                
                    readF_command readF;
                    strcpy(readF.file,words[1]);
                    if(num_words>2) readF.line_number = atoi(words[2]);
                    else readF.line_number = -1;
                    printf("File1: %s\n", readF.file);
                    //Put \0 char to the end of the command
                    // command[num_read] = '\0';
                    // printf("Command: %s\n", command[0]);
                    if (num_read == -1)
                    {
                        perror("Failed to read from server request FIFO");
                        close(server_fd);
                        close(client_res_fd);
                        close(server_req_fd);
                        exit(EXIT_FAILURE);
                    }
                    /* */

                    // int response = 1;
                    // ssize_t num_written = write(client_res_fd, &response, sizeof(response));
                    printf("readf command\n");
                    // readF_command readF;
                    // ssize_t num_read = read(server_req_fd, &readF, sizeof(readF));
                    // if (num_read == -1)
                    // {
                    //     perror("Failed to read from server request FIFO");
                    //     close(server_fd);
                    //     close(client_res_fd);
                    //     close(server_req_fd);
                    //     exit(EXIT_FAILURE);
                    // }

                    enterRegionReader(getSafeFile(dir_syncs, readF.file));

                    // File name is the directory of server + file name
                    char file_path[512];
                    sprintf(file_path, "%s/%s", dirname, readF.file);

                    FILE *file = fopen(file_path, "r");
                    printf("File2: %s\n", file_path);
                    if (file == NULL)
                    {
                        perror("Failed to open file");
                        close(server_fd);
                        close(client_res_fd);
                        close(server_req_fd);
                        exitRegionReader(getSafeFile(dir_syncs, readF.file));
                        exit(EXIT_FAILURE);
                    }

                    // If the user wants a specific line
                    if(readF.line_number!=-1)
                    {
                        printf("Line number: %d\n", readF.line_number);
                        char line[256];
                        int line_number = 0;
                        while (fgets(line, sizeof(line), file) != NULL)
                        {
                            line_number++;
                            if (line_number == readF.line_number)
                            {
                                break;
                            }
                        }

                        if (line_number != readF.line_number)
                        {
                            const char *error_message = "Line number is out of range";
                            if (write(client_res_fd, error_message, strlen(error_message)) == -1)
                            {
                                perror("Failed to write error message to client response FIFO");
                                close(server_fd);
                                close(client_res_fd);
                                close(server_req_fd);
                                exitRegionReader(getSafeFile(dir_syncs, readF.file));
                                exit(EXIT_FAILURE);
                            }
                        }
                        else
                        {
                            // OPen the named semaphore "readyToRead"
                            // sem_t *sem = sem_open("readyToRead", O_CREAT, 0666, 0);

                            if (write(client_res_fd, line, strlen(line)) == -1)
                            {
                                perror("Failed to write to client response FIFO");
                                close(server_fd);
                                close(client_res_fd);
                                close(server_req_fd);
                                exitRegionReader(getSafeFile(dir_syncs, readF.file));
                                exit(EXIT_FAILURE);
                            }
                            printf("Writing is ok:");
                            
                            //Up the semaphore
                            // sem_post(sem);
                        }

                        fclose(file);
                        close(client_res_fd);
                        client_res_fd = -1;
                        exitRegionReader(getSafeFile(dir_syncs, readF.file));

                        // read from fifo
                        int garbage;
                        ssize_t num_read = read(server_req_fd, &garbage, sizeof(int));
                        printf("num_read: %zd\n", num_read);
                         // if it is closed, open it: client_res_fd
                        if (client_res_fd == -1)
                        {
                            printf("Client response FIFO is closed. Reopening...\n");
                            client_res_fd = open(client_res_fifo, O_WRONLY);
                            if (client_res_fd == -1)
                            {
                                perror("Failed to open client response FIFO");
                                close(server_fd);
                                close(client_res_fd);
                                close(server_req_fd);
                                exit(EXIT_FAILURE);
                            }
                        }

                    }
                    
                    // If the user wants the whole file}
                    else
                    {
                        printf("Whole file\n");
                        char line[256];
                        while (fgets(line, sizeof(line), file) != NULL)
                        {
                            if (write(client_res_fd, line, strlen(line)) == -1)
                            {
                                perror("Failed to write to client response FIFO");
                                close(server_fd);
                                close(client_res_fd);
                                close(server_req_fd);
                                exitRegionReader(getSafeFile(dir_syncs, readF.file));
                                exit(EXIT_FAILURE);
                            }
                        }

                        fclose(file);
                        close(client_res_fd);
                        client_res_fd = -1;
                        exitRegionReader(getSafeFile(dir_syncs, readF.file));
                    }
                    

                    printf("eof is reached\n");
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
