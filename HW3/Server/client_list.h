#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H
#define QUEUE_SIZE_ 60

struct client_list_wrapper
{
    int counter;
    int clients[QUEUE_SIZE_];
};

/**
 * Initializes the client list.
 * @param client_list The client list to initialize.
 * @return void
*/
void initialize_client_list(struct client_list_wrapper *client_list);

/**
 * Adds a client to the client list.
 * @param client_list The client list to add the client to.
 * @param client_pid The PID of the client to add.
 * @return void
*/
void add_client(struct client_list_wrapper *client_list, int client_pid);

/**
 * Removes a client from the client list.
 * @param client_list The client list to remove the client from.
 * @param client_pid The PID of the client to remove.
 * @return void
*/
void remove_client(struct client_list_wrapper *client_list, int client_pid);

/**
 * Checks if a client is in the client list.
 * @param client_list The client list to check.
 * @param client_pid The PID of the client to check.
 * @return 1 if the client is in the list, 0 otherwise.
*/
int is_client_in_list(struct client_list_wrapper *client_list, int client_pid);

/**
 * Frees the client list.
 * @param client_list The client list to free.
 * @return void
*/
void free_client_list(struct client_list_wrapper *client_list);

/**
 * Traverses the client list and kills all child processes.
 * @param client_list The client list to traverse.
 * @return void
*/
void traverse_and_kill_children(struct client_list_wrapper *client_list);


/**
 * Prints all clients in the client list.
 * @param client_list The client list to print.
 * @return void
*/
void print_all_clients(struct client_list_wrapper *client_list);

/**
 * Get the client at the specified index.
 * @param client_list The client list to get the client from.
 * @param index The index of the client to get.
 * @return The client at the specified index.
*/
int get_client(struct client_list_wrapper *client_list, int index);


/**
 * Get the size of the client list.
 * @param client_list The client list to get the size of.
 * @return The size of the client list.
*/
int get_client_list_size(struct client_list_wrapper *client_list);

#endif