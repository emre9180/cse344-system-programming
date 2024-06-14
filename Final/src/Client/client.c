#include "../../include/Client/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

void initialize_clients(int num_clients, char *server_ip, int p, int q)
{
    clients = (Client *)malloc(num_clients * sizeof(Client));
    if (clients == NULL)
    {
        perror("Failed to allocate memory for clients");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    for (int i = 0; i < num_clients; i++)
    {
        memset(&clients[i], 0, sizeof(Client)); // Ensure all members are initialized

        clients[i].client_id = i;
        clients[i].socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (clients[i].socket_fd < 0)
        {
            perror("Error creating socket");
            free(clients);
            exit(EXIT_FAILURE);
        }

        clients[i].server_addr.sin_family = AF_INET;
        clients[i].server_addr.sin_port = htons(8080);
        if (inet_pton(AF_INET, server_ip, &clients[i].server_addr.sin_addr) <= 0)
        {
            perror("Invalid address/Address not supported");
            free(clients);
            exit(EXIT_FAILURE);
        }

        clients[i].x = rand() % p;
        clients[i].y = rand() % q;
        clients[i].p = p;
        clients[i].q = q;
    }
}

void *client_function(void *arg)
{
    Client *client = (Client *)arg;

    // Create a socket for this client thread
    client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_fd < 0)
    {
        perror("Error creating socket");
        pthread_exit(NULL);
    }

    // printf("Client %d trying to connect to the server at %s:%d\n", client->client_id,
    //        inet_ntoa(client->server_addr.sin_addr), ntohs(client->server_addr.sin_port));

    if (connect(client->socket_fd, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)) < 0)
    {
        perror("Connection Failed");
        close(client->socket_fd);
        pthread_exit(NULL);
    }

    // printf("Client %d connected to the server at %s:%d\n", client->client_id,
    //        inet_ntoa(client->server_addr.sin_addr), ntohs(client->server_addr.sin_port));

    send_order(client);

    // Read the server response

    char buffer[BUFFER_SIZE] = {0};
    while (keep_running)
    {
        int bytes_received = recv(client->socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            if (errno == EBADF || errno == EINTR) // Check for invalid socket descriptor or interrupted system call
            {
                break;
            }
            perror("Error reading from server");
            break;
        }
        if (bytes_received == 0)
        {
            // printf("Server closed the connection\n");
            break;
        }

        buffer[bytes_received] = '\0';

        // Check for the specific response indicating all customers are served
        if (strstr(buffer, "Your order is delivered.") != NULL)
        {
            printf("Client %d received: %s\n", client->client_id, buffer);
            break; // Exit the loop
        }
        else
        {
            printf("Client %d received: %s\n", client->client_id, buffer);
        }
    }
    // printf("Client %d closing connection\n", client->client_id);
    pthread_exit(NULL);
}

void client_function2(const char *server_ip)
{
    Client client;

    client.client_id = -1;
    client.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client.socket_fd < 0)
    {
        perror("Error creating socket");
        free(clients);
        exit(EXIT_FAILURE);
    }

    client.server_addr.sin_family = AF_INET;
    client.server_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, server_ip, &client.server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/Address not supported");
        free(clients);
        exit(EXIT_FAILURE);
    }

    // Create a socket for this client thread
    client.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client.socket_fd < 0)
    {
        perror("Error creating socket");
        pthread_exit(NULL);
    }

    // printf("Client %d trying to connect to the server at %s:%d\n", client.client_id,
    //        inet_ntoa(client.server_addr.sin_addr), ntohs(client.server_addr.sin_port));

    if (connect(client.socket_fd, (struct sockaddr *)&client.server_addr, sizeof(client.server_addr)) < 0)
    {
        perror("Connection Failed");
        close(client.socket_fd);
        pthread_exit(NULL);
    }

    // printf("Client %d connected to the server at %s:%d\n", client.client_id,
    //        inet_ntoa(client.server_addr.sin_addr), ntohs(client.server_addr.sin_port));

    char buffer[BUFFER_SIZE] = {0};
    snprintf(buffer, sizeof(buffer), "-1 -1 -1 -1 -1 -1 -1 -1 -1");
    if (send(client.socket_fd, buffer, strlen(buffer), 0) < 0)
    {
        perror("Error sending disconnection message");
    }
    while (keep_running)
    {
        int bytes_received = recv(client.socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            if (errno == EBADF || errno == EINTR) // Check for invalid socket descriptor or interrupted system call
            {
                break;
            }
            perror("Error reading from server");
            break;
        }
        if (bytes_received == 0)
        {
            // printf("Server closed the connection\n");
            break;
        }

        buffer[bytes_received] = '\0';

        // Check for the specific response indicating all customers are served
        if (strstr(buffer, "Your order is delivered.") != NULL)
        {
            // printf("Client %d received: %s\n", client.client_id, buffer);
            break; // Exit the loop
        }
        else
        {
            // printf("Client %d received: %s\n", client.client_id, buffer);
        }
    }
}

void send_order(Client *client)
{
    char buffer[BUFFER_SIZE] = {0};
    if (keep_running)
    {
        // Serialize the client struct into the buffer
        snprintf(buffer, sizeof(buffer), "%d %d %s %d %d %s %d %d %d",
                 client->client_id,
                 client->x,
                 inet_ntoa(client->server_addr.sin_addr),
                 ntohs(client->server_addr.sin_port),
                 client->y,
                 inet_ntoa(client->client_addr.sin_addr),
                 ntohs(client->client_addr.sin_port),
                 client->p,
                 client->q

        );
    }
    else
    {
        // Send -1 -1 to indicate disconnection
        snprintf(buffer, sizeof(buffer), "-1 -1 -1 -1 -1 -1 -1 -1 -1");
    }
    send(client->socket_fd, buffer, strlen(buffer), 0);
}

void cleanup()
{
    if (clients != NULL)
    {
        for (int i = 0; i < num_clients; i++)
        {
            close(clients[i].socket_fd);
        }
        free(clients);
        // printf("Cleaned up client connections\n");
    }
}
