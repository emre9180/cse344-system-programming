#include "../../include/Server/server_connection.h"
#include "../../include/Server/pide_house.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// Global variables
int p = 1;
int q = 1;
int server_socket_fd;
ClientConnection *client_connections[MAX_CONNECTIONS];
int num_connections = 0;
pthread_mutex_t connection_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t *client_threads[MAX_CONNECTIONS];
int client_thread_count = 0;

int initialize_server(int port, const char *ip_address)
{
    struct sockaddr_in server_addr;

    // Create socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/Address not supported");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // Bind
    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket_fd, MAX_CONNECTIONS) < 0)
    {
        perror("Error listening on socket");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s:%d\n", ip_address, port);
    return server_socket_fd;
}

// Handle client connection
void *handle_client(void *arg)
{
    ClientConnection *client_connection = (ClientConnection *)arg;
    int client_socket = client_connection->socket_fd;
    char buffer[BUFFER_SIZE] = {0};
    int bytes_read;

    // printf("Accepted connection");

    // Read client coordinates
    bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read < 0)
    {
        perror("Error reading from client socket");
        close(client_socket);
        pthread_exit(NULL);
    }
    // Parse client data
    int client_id, x, y;
    char server_ip[INET_ADDRSTRLEN], client_ip[INET_ADDRSTRLEN];
    int server_port, client_port;
    // Parse coordinates
    if (sscanf(buffer, "%d %d %s %d %d %s %d %d %d",
               &client_id,
               &x,
               server_ip,
               &server_port,
               &y,
               client_ip,
               &client_port,
               &p,
               &q) != 9)
    {
        fprintf(stderr, "Error parsing client data: %s\n", buffer);
        close(client_socket);
        pthread_exit(NULL);
    }
    if (client_id == -1)
    {
        char buffer[256] = {0};
        sprintf(buffer, "Client disconnected\n");
        printf("%s", buffer);
        writeLog(buffer);
        cancel_order_flag = 1;

        // Notify all condition variables to wake up waiting threads
        pthread_cond_broadcast(&order_cond);
        pthread_cond_broadcast(&cooked_cond);
        pthread_cond_broadcast(&oven_cond);
        pthread_cond_broadcast(&delivery_cond);
        pthread_cond_broadcast(&delivery_bag_cond);
        pthread_cond_broadcast(&motor_cond);
        pthread_cond_broadcast(&apparatus_cond);
        pthread_cond_broadcast(&oven_putting_opening_cond);
        pthread_cond_broadcast(&oven_removing_opening_cond);

        for (int i = 0; i < num_cooks; i++)
        {
            pthread_cond_broadcast(&cooks[i].cookCond);
        }
        for (int i = 0; i < num_delivery_persons; i++)
        {
            pthread_cond_broadcast(&delivery_persons[i].order_bag->cond);
        }

        pthread_join(manager_thread, NULL);

        for (int i = 0; i < num_cooks; i++)
        {
            pthread_join(cook_threads[i], NULL);
        }

        for (int i = 0; i < num_delivery_persons; i++)
        {
            pthread_join(delivery_threads[i], NULL);
        }

        // destroy mutexes and conds of delivery and cooks
        for (int i = 0; i < num_delivery_persons; i++)
        {
            pthread_mutex_destroy(&delivery_persons[i].order_bag->mutex);
            pthread_cond_destroy(&delivery_persons[i].order_bag->cond);
        }

        // clean cook's mutex and conds
        for (int i = 0; i < num_cooks; i++)
        {
            pthread_mutex_destroy(&cooks[i].cookMutex);
            pthread_cond_destroy(&cooks[i].cookCond);
        }

        // printStatistics();

        // Clean cooks' all orders
        for (int i = 0; i < num_cooks; i++)
        {
            while (dequeue(cooks[i].order_queue) != NULL)
                ;
        }

        // Clean delivery persons' all orders
        for (int i = 0; i < num_delivery_persons; i++)
        {
            while (dequeue(delivery_persons[i].order_bag) != NULL)
                ;
        }

        // free order queue and bag of cooks and delivery persons
        for (int i = 0; i < num_cooks; i++)
        {
            free(cooks[i].order_queue);
        }

        for (int i = 0; i < num_delivery_persons; i++)
        {
            free(delivery_persons[i].order_bag);
        }
        // Clean orders linked list
        ListNode *current = orders.head;
        while (current != NULL)
        {
            ListNode *temp = current;
            current = current->next;
            free(temp);
        }
        orders.head = NULL;
        cancel_order_flag = 0;

        initialize_system(num_cooks, num_delivery_persons, speed);
        printf("Temizlendi cikis yapiliyor...\n");
        fflush(stdout);

        close(client_socket);
        pthread_exit(NULL);
    }

    // Populate the client_connection struct
    client_connection->x = x;
    client_connection->y = y;
    inet_pton(AF_INET, client_ip, &client_connection->client_addr.sin_addr);
    client_connection->client_addr.sin_port = htons(client_port);

    // Create a new order
    Order order;
    order.customer_id = client_socket; // Use socket fd as customer ID for simplicity
    order.x = client_connection->x;
    order.y = client_connection->y;
    order.preparation_time = (double)rand() / RAND_MAX * 5.0 + 1.0; // Example preparation time
    order.cooking_time = order.preparation_time / 2.0;              // Example cooking time
    order.delivery_time = 0;                                        // Simplified delivery time
    order.is_cancelled = 0;
    order.delivery_person_id = -1;
    order.cook_id = -1;
    order.ready = 0;
    order.taken = 0;
    order.socket_fd = client_socket;

    // Place order
    place_order(&order);

    // Send response to client
    send_response(client_socket, "Order received and being processed.\n");

    // Close connection
    // close_connection(client_socket);
    pthread_exit(NULL);
}

// Send response to client
void send_response(int client_socket, const char *response)
{
    send(client_socket, response, strlen(response), 0);
}

// Close client connection
void close_connection(int client_socket)
{
    pthread_mutex_lock(&connection_mutex);
    for (int i = 0; i < num_connections; i++)
    {
        if (client_connections[i]->socket_fd == client_socket)
        {
            close(client_connections[i]->socket_fd);
            client_connections[i] = client_connections[num_connections - 1];
            num_connections--;
            break;
        }
    }
    pthread_mutex_unlock(&connection_mutex);
}

// Shutdown server
void shutdown_server()
{
    close(server_socket_fd);
    for (int i = 0; i < num_connections; i++)
    {
        close(client_connections[i]->socket_fd);
    }
}
