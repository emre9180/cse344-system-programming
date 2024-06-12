#include "../../include/Client/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Global variables
Client *clients = NULL;
int num_clients = 0;
volatile sig_atomic_t keep_running = 1;

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <num_clients> <p> <q>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    num_clients = atoi(argv[2]);
    int p = atoi(argv[3]);
    int q = atoi(argv[4]);

    // Setup signal handler
    setup_signal_handler();

    initialize_clients(num_clients, server_ip, p, q);

    pthread_t *threads = (pthread_t *)malloc(num_clients * sizeof(pthread_t));
    for (int i = 0; i < num_clients; i++) {
        pthread_create(&threads[i], NULL, client_function, (void *)&clients[i]);
    }

    for (int i = 0; i < num_clients; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    cleanup();
    printf("All customers served\n");

    return 0;
}

// Signal handler function
void handle_signal(int sig) {
    printf("\nCaught signal %d (SIGINT), cleaning up...\n", sig);
    keep_running = 0;
    for (int i = 0; i < num_clients; i++) {
        // Send -1 -1 to indicate disconnection
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "-1 -1");
        send(clients[i].socket_fd, buffer, strlen(buffer), 0);
    }
    cleanup();
    exit(EXIT_SUCCESS);
}

// Setup signal handler using sigaction
void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
}
