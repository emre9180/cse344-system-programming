#include "../../include/Client/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

// Global variables
Client *clients = NULL;
int num_clients = 0;
volatile sig_atomic_t keep_running = 1;
char server_ip[32];
char port[32];
pthread_t *threads;

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port> <num_clients> <p> <q>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_clients = atoi(argv[3]);
    int p = atoi(argv[4]);
    int q = atoi(argv[5]);
    strcpy(server_ip, argv[1]);
    strcpy(port, argv[2]);
    printf("Server IP: %s", server_ip);
    printf(":%s\n", port);

    // Setup signal handler
    setup_signal_handler();

    initialize_clients(num_clients, server_ip, port, p, q);

    threads = (pthread_t *)malloc(num_clients * sizeof(pthread_t));
    for (int i = 0; i < num_clients; i++)
    {
        pthread_create(&threads[i], NULL, client_function, (void *)&clients[i]);
    }

    for (int i = 0; i < num_clients; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    cleanup();
    printf("Client generator program will be closed successfuly. You can start it again!\n");
    client_function2(server_ip);
    return 0;
}

// Signal handler function
void handle_signal(int sig)
{
    printf("\nCaught signal %d (SIGINT), cleaning up... Client generator program will be closed. You can start it again!\n", sig);
    keep_running = 0;
    client_function2(server_ip);



    for (int i = 0; i < num_clients; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // DEstroy sockets
    free(threads);
    cleanup();
    exit(EXIT_SUCCESS);
}

// Setup signal handler using sigaction
void setup_signal_handler()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
}
