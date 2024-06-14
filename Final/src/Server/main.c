#include "../../include/Server/pide_house.h"
#include "../../include/Server/server_connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Global variable to store the server socket
int server_socket;
// Global flag for shutdown


// Signal handler function
void handle_sigint(int sig) {
    printf("\nCaught signal %d (SIGINT), closing the server socket...\n", sig);
    shutdown_flag = 1;

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
                
    for (int i = 0; i < num_cooks; i++) {
        pthread_cond_broadcast(&cooks[i].cookCond);
    }
    for (int i = 0; i < num_delivery_persons; i++) {
        pthread_cond_broadcast(&delivery_persons[i].order_bag->cond);
    }
    
        // send signal to ooks
    for (int i = 0; i < num_cooks; i++)
    {
        pthread_cond_signal(&cooks[i].cookCond);
    }
    printf("Waiting for all threads to finish...\n");

    for (int i = 0; i < num_cooks; i++) {
        pthread_join(cook_threads[i], NULL);
    }

    for (int i = 0; i < num_delivery_persons; i++) {
        pthread_join(delivery_threads[i], NULL);
    }

    pthread_join(manager_thread, NULL);
    // Free delivery person's mutexes and cond vars
    for (int i = 0; i < num_delivery_persons; i++) {
        pthread_mutex_destroy(&delivery_persons[i].order_bag->mutex);
        pthread_cond_destroy(&delivery_persons[i].order_bag->cond);
    }

    // Free cook's mutexes and cond vars
    for (int i = 0; i < num_cooks; i++) {
        pthread_mutex_destroy(&cooks[i].cookMutex);
        pthread_cond_destroy(&cooks[i].cookCond);
    }

    // Free delivery men
    // Free nodes in delivery man's order bag
    for (int i = 0; i < num_delivery_persons; i++) {
        QueueNode *current = delivery_persons[i].order_bag->front;
        while (current) {
            QueueNode *temp = current;
            current = current->next;
            free(temp);
        }
    }

    for (int i = 0; i < num_delivery_persons; i++) {
        free(delivery_persons[i].order_bag);
    }

    // Free nodes in cook's order bag
    for (int i = 0; i < num_cooks; i++) {
        QueueNode *current = cooks[i].order_queue->front;
        while (current) {
            QueueNode *temp = current;
            current = current->next;
            free(temp);
        }
    }

    // Free cooks
    for (int i = 0; i < num_cooks; i++) {
        free(cooks[i].order_queue);
    }

    free(cook_threads);
    free(delivery_threads);

    

    
    

    // Close mutexes and cond variablesr
    pthread_mutex_destroy(&order_mutex);
    pthread_mutex_destroy(&delivery_mutex);
    pthread_mutex_destroy(&cook_mutex);
    pthread_mutex_destroy(&oven_mutex);
    pthread_mutex_destroy(&delivery_bag_mutex);
    pthread_mutex_destroy(&motor_mutex);
    pthread_mutex_destroy(&apparatus_mutex);
    pthread_mutex_destroy(&oven_putting_opening_mutex);
    pthread_mutex_destroy(&oven_removing_opening_mutex);

    pthread_cond_destroy(&order_cond);
    pthread_cond_destroy(&cooked_cond);
    pthread_cond_destroy(&oven_cond);
    pthread_cond_destroy(&delivery_cond);
    pthread_cond_destroy(&delivery_bag_cond);
    pthread_cond_destroy(&motor_cond);
    pthread_cond_destroy(&apparatus_cond);
    pthread_cond_destroy(&oven_putting_opening_cond);
    pthread_cond_destroy(&oven_removing_opening_cond);

    printf("All threads finished. Cleaning up...\n");
    fflush(stdout);

    for (int i = 0; i < num_connections; i++) {
        close(client_connections[i]->socket_fd);
    }

    for(int i = 0; i < num_connections; i++) {
        free(client_connections[i]);
    }

    for (int i = 0; i < client_thread_count; i++) {
        pthread_join(*client_threads[i], NULL);
    }

    for (int i = 0; i < client_thread_count; i++) {
        free(client_threads[i]);
    }

    // Traverse linked list orders and free each node
    ListNode *current = orders.head;
    while (current != NULL) {
        ListNode *temp = current;
        current = current->next;
        free(temp);
    }


    // Close the server socket to stop accepting new connections
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
    initialize_system(n_cooks, m_delivery_persons, delivery_speed);

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

        // printf("Accepted connection from %d:%d\n", inet_ntoa(client_connection->client_addr.sin_addr), ntohs(client_connection->client_addr.sin_port));

        pthread_mutex_lock(&connection_mutex);
        if (num_connections < MAX_CONNECTIONS) {
            client_connections[num_connections++] = client_connection;
            client_threads[client_thread_count++] = (pthread_t *)malloc(sizeof(pthread_t));
            pthread_create(client_threads[client_thread_count-1], NULL, handle_client, client_connection);
            // pthread_join(*client_threads[client_thread_count-1], NULL);
            // free(client_threads[client_thread_count-1]);
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
