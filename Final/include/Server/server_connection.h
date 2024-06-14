#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>

// Define constants
#define SERVER_PORT 8080  // Fixed port for server
#define MAX_CONNECTIONS 1000
#define BUFFER_SIZE 1024

// Structure to represent a client connection
typedef struct {
    int socket_fd;
    struct sockaddr_in client_addr;
    int x; // x-coordinate of the client
    int y; // y-coordinate of the client
} ClientConnection;

// Global variables
extern int server_socket_fd;
extern ClientConnection* client_connections[MAX_CONNECTIONS];
extern int num_connections;
extern pthread_mutex_t connection_mutex;
extern pthread_t* client_threads[MAX_CONNECTIONS];
extern int client_thread_count;
extern int p;
extern int q;

// Function prototypes
int initialize_server(int port, const char *ip_address);
void* handle_client(void *arg);
void send_response(int client_socket, const char *response);
void close_connection(int client_socket);
void shutdown_server();

#endif // CONNECTION_H
