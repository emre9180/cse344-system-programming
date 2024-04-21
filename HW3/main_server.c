#include "common.h"
#include "client_info_queue.h"

// Function prototypes
void handle_signal(int sig);
void setup_signal_handler();
void server_loop(const char* dirname, int max_clients);

// Global variables
char log_filename[256];
Semaphores *sems;
struct queue client_queue;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dirname> <max.#ofClients>\n", argv[0]);
        exit(1);
    }

    char *dirname = argv[1];
    int max_clients = atoi(argv[2]);

    create_named_pipe(SERVER_FIFO);

    printf("Server Started PID %d...\n", getpid());

    sems = initialize_shared_memory();

    setup_signal_handler();

    server_loop(dirname, max_clients);

    cleanup_shared_memory(sems);

    return 0;
}

void handle_signal(int sig) {
    if (sig == SIGINT) {
        const char *msg = "Received SIGINT. Server terminating...\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        char server_fifo[256];
        sprintf(server_fifo, SERVER_FIFO, getpid());
        sem_destroy(&sems->mutex);
        sem_destroy(&sems->empty);
        sem_destroy(&sems->full);
        unlink(server_fifo);
        cleanup_shared_memory(sems);
        exit(0);
    } else if (sig == SIGCHLD) {
        cleanup_child_processes(sems);
    }
}

void setup_signal_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = SA_RESTART;  // Restart sys-calls automatically
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

void server_loop(const char* dirname, int max_clients) {
    char *server_fifo = (char *)malloc(256);
    int server_pid = getpid();
    sprintf(server_fifo, SERVER_FIFO, server_pid);

    mkdir(dirname, 0755);

    create_named_pipe(server_fifo);

    int server_fd = open(server_fifo, O_RDONLY);
    if (server_fd == -1) {
        perror("Failed to open server FIFO");
        exit(1);
    }

    while (1) {
        struct client_info cli_info;
        ssize_t num_read = read(server_fd, &cli_info, sizeof(cli_info));
        if (num_read == -1) {
            perror("Failed to read from server FIFO");
            close(server_fd);
            exit(EXIT_FAILURE);
        } else if (num_read == 0 && is_queue_empty(&client_queue)) {
            continue;
        } else if (num_read != 0) {
            printf("Received data: PID=%d, Data=%s, bytes:%d\n", cli_info.pid, cli_info.cwd, num_read);
            enqueue(&client_queue, cli_info);
            printf("Client info enqueued\n");
            printf("Client PID: %d\n", cli_info.pid);
            printf("Client CWD: %s\n", cli_info.cwd);
            printf("Client wait: %d\n", cli_info.wait);
        }

        if (sem_trywait(&sems->empty) != 0) {
            if (errno == EAGAIN) {
                printf("Queue is full. Please wait for connection...\n");
                sem_wait(&sems->empty);
                printf("Queue is free. You are connecting...\n");
            } else {
                perror("Failed to acquire semaphore");
                sem_destroy(&sems->empty);
                return;
            }
        }

        cli_info = dequeue(&client_queue);
        printf("Successfuly connected!: %d\n", cli_info.pid);

        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("Failed to fork child process");
            exit(1);
        }

        if (child_pid == 0) {
            printf("Child execution...\n");
            sleep(5);

            exit(0);
        } else {
            printf("Paren loop continue...\n");
            continue;
        }
    }

    close(server_fd);
    unlink(server_fifo);
}
