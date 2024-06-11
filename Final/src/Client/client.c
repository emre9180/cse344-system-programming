#include "../../include/Client/client.h"


// Global variables
Client *clients = NULL;
int num_clients = 0;
volatile sig_atomic_t keep_running = 1;
void initialize_clients(int num_clients, const char *server_ip, int p, int q) {
    clients = (Client *)malloc(num_clients * sizeof(Client));
    if (clients == NULL) {
        perror("Failed to allocate memory for clients");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    for (int i = 0; i < num_clients; i++) {
        clients[i].client_id = i;
        clients[i].socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (clients[i].socket_fd < 0) {
            perror("Error creating socket");
            free(clients);
            exit(EXIT_FAILURE);
        }

        clients[i].server_addr.sin_family = AF_INET;
        clients[i].server_addr.sin_port = htons(8080);
        if (inet_pton(AF_INET, server_ip, &clients[i].server_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            free(clients);
            exit(EXIT_FAILURE);
        }

        clients[i].x = rand() % p;
        clients[i].y = rand() % q;
    }
}


void* client_function(void *arg) {
    Client *client = (Client *)arg;

    // show which ip and port we are trying to connect
    printf("Client %d trying to connect to the server at %s:%d\n", client->client_id,
           inet_ntoa(client->server_addr.sin_addr), ntohs(client->server_addr.sin_port));
    

    if (connect(client->socket_fd, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)) < 0) {
        perror("Connection Failed");
        pthread_exit(NULL);
    }

    printf("Client %d connected to the server at %s:%d\n", client->client_id,
           inet_ntoa(client->server_addr.sin_addr), ntohs(client->server_addr.sin_port));

    send_order(client);

    close(client->socket_fd);
    pthread_exit(NULL);
}

void send_order(Client *client) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%d %d", client->x, client->y);
    send(client->socket_fd, buffer, strlen(buffer), 0);
    read(client->socket_fd, buffer, sizeof(buffer));
    printf("Client %d received: %s\n", client->client_id, buffer);
}

void handle_signal(int signal) {
    keep_running = 0;
    cleanup();
    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (clients != NULL) {
        for (int i = 0; i < num_clients; i++) {
            close(clients[i].socket_fd);
        }
        free(clients);
        printf("Cleaned up client connections\n");
    }
}