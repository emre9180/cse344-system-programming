#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#include <netinet/in.h>
#include <signal.h>

// Define constants
#define SERVER_PORT 192  // Assuming a fixed port, change if necessary
#define BUFFER_SIZE 1024

// Structure to represent a client
typedef struct {
    int client_id;
    int socket_fd;
    struct sockaddr_in server_addr;
    int x; // x-coordinate of the client
    int y; // y-coordinate of the client
    struct sockaddr_in client_addr;
    int p;
    int q;
} Client;

// Global variables
extern Client *clients;
extern int num_clients;
extern volatile sig_atomic_t keep_running;

// Function prototypes
void initialize_clients(int num_clients, const char *server_ip, int p, int q);
void* client_function(void *arg);
void send_order(Client *client);
void handle_signal(int sig);
void setup_signal_handler();
void cleanup();
void create_sigint_client(const char *server_ip);

#endif // CLIENT_H