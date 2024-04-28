#ifndef CLIENT_INFO_QUEUE_H
#define CLIENT_INFO_QUEUE_H

#include "common.h"
#define QUEUE_SIZE 10

struct client_info
{
    pid_t pid;
    int mode;
    char cwd[CWD_SIZE];
};

struct queue
{
    struct client_info data[QUEUE_SIZE];
    int front;
    int rear;
    int count;
};

// Initializes the queue
void initialize_queue(struct queue *q);

// Checks if the queue is empty
int is_queue_empty(struct queue *q);

// Checks if the queue is full
int is_queue_full(struct queue *q);

// Adds an item to the queue
void enqueue(struct queue *q, struct client_info item);

// Removes and returns an item from the queue
struct client_info dequeue(struct queue *q);

// Returns the size of the queue
int get_queue_size(struct queue *q);

#endif