#include "client_list.h"
#include "signal.h"
#include <stdlib.h>
#include <stdio.h>

void initialize_client_list(struct client_list_wrapper *client_list)
{
    client_list->counter = 0;
    // client_list->clients = (int *)((char *)client_list + sizeof(int) + sizeof(int *));
}

void add_client(struct client_list_wrapper *client_list, int client_pid)
{
    client_list->clients[client_list->counter] = client_pid;
    client_list->counter++;
}

void remove_client(struct client_list_wrapper *client_list, int client_pid)
{
    int i;
    for (i = 0; i < client_list->counter; i++)
    {
        if (client_list->clients[i] == client_pid)
        {
            // Shift elements to the left
            for (int j = i; j < client_list->counter - 1; j++)
            {
                client_list->clients[j] = client_list->clients[j + 1];
            }
            client_list->clients[client_list->counter - 1] = 0;
            client_list->counter--;
            break;
        }
    }
}

int is_client_in_list(struct client_list_wrapper *client_list, int client_pid)
{
    int i;
    for (i = 0; i < client_list->counter; i++)
    {
        if (client_list->clients[i] == client_pid)
        {
            return 1;
        }
    }
    return 0;
}

void free_client_list(struct client_list_wrapper *client_list)
{
    free(client_list->clients);
}

void print_all_clients(struct client_list_wrapper *client_list)
{
    int i;
    for (i = 0; i < client_list->counter; i++)
    {
        printf("Client PID: %d\n", client_list->clients[i]);
    }
}

void traverse_and_kill_children(struct client_list_wrapper *client_list)
{
    int i;
    for (i = 0; i < client_list->counter; i++)
    {
        kill(client_list->clients[i], SIGINT);
    }
}

int get_client(struct client_list_wrapper *client_list, int index)
{
    return client_list->clients[index];
}

int get_client_list_size(struct client_list_wrapper *client_list)
{
    return client_list->counter;
}