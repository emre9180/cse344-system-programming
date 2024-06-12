#include "../../include/Server/pide_house.h"
#include "../../include/Server/server_connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Global variable to store the server socket
int server_socket;

// Signal handler function
void handle_sigint(int sig) {
    printf("\nCaught signal %d (SIGINT), closing the server socket...\n", sig);
    close(server_socket);
    exit(EXIT_SUCCESS);
}

// Setup signal handler
void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <ip_address> <n_cooks> <m_delivery_persons> <delivery_speed>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int n_cooks = atoi(argv[2]);
    int m_delivery_persons = atoi(argv[3]);
    int delivery_speed = atoi(argv[4]);

    // Initialize the pide house system
    initialize_system(n_cooks, m_delivery_persons);

    // Setup the signal handler
    setup_signal_handler();

    // Initialize server
    server_socket = initialize_server(8080, ip_address);
    if (server_socket < 0) {
        fprintf(stderr, "Failed to initialize server\n");
        exit(EXIT_FAILURE);
    }

    // Accept connections
    while (1) {
        ClientConnection *client_connection = (ClientConnection *)malloc(sizeof(ClientConnection));
        if (!client_connection) {
            perror("Failed to allocate memory for client connection");
            continue;
        }
        socklen_t client_len = sizeof(client_connection->client_addr);

        // Accept new connection
        client_connection->socket_fd = accept(server_socket, (struct sockaddr *)&client_connection->client_addr, &client_len);
        if (client_connection->socket_fd < 0) {
            perror("Error accepting connection");
            free(client_connection);
            continue;
        }

        // printf("Accepted connection from %s:%d\n", inet_ntoa(client_connection->client_addr.sin_addr), ntohs(client_connection->client_addr.sin_port));

        pthread_mutex_lock(&connection_mutex);
        if (num_connections < MAX_CONNECTIONS) {
            client_connections[num_connections++] = *client_connection;
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handle_client, client_connection);
            pthread_detach(client_thread);
        } else {
            send_response(client_connection->socket_fd, "Server is full. Try again later.\n");
            // close(client_connection->socket_fd);
            free(client_connection);
        }
        pthread_mutex_unlock(&connection_mutex);
    }
    // Shutdown server (this code is not expected to be reached in this simple example)
    shutdown_server();
    return 0;
}
