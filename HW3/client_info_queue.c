#include "client_info_queue.h"

// Initialize the queue
void initialize_queue(struct queue *q)
{
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

// Check if the queue is empty
int is_queue_empty(struct queue *q)
{
    return q->count == 0;
}

// Check if the queue is full
int is_queue_full(struct queue *q)
{
    return q->count == QUEUE_SIZE;
}

// Enqueue an item into the queue
void enqueue(struct queue *q, struct client_info item)
{
    if (is_queue_full(q))
    {
        printf("Queue is full. Cannot enqueue.\n");
        return;
    }
    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->data[q->rear] = item;
    q->count++;
}

// Dequeue an item from the queue
struct client_info dequeue(struct queue *q)
{
    if (is_queue_empty(q))
    {
        printf("Queue is empty. Cannot dequeue.\n");
        struct client_info empty_client_info;
        empty_client_info.pid = -1;
        return empty_client_info;
    }
    struct client_info item = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->count--;
    return item;
}
// Get the size of the queue
int get_queue_size(struct queue *q)
{
    return q->count;
}
