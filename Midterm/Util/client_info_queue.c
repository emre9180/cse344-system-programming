#include "client_info_queue.h"

// Initialize the queue
void initialize_queue(struct queue *q) {
    q->front = 0;
    q->rear = 0;  // Start from 0, and adjust enqueue to not increment before the first use
    q->count = 0;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        q->data[i].pid = -1;  // Initialize with a default invalid PID
    }
}

// Check if the queue is empty
int is_queue_empty(struct queue *q) {
    return q->count == 0;
}

// Check if the queue is full
int is_queue_full(struct queue *q) {
    return q->count == QUEUE_SIZE;
}

// Enqueue an item into the queue
void enqueue(struct queue *q, struct client_info item) {
    if (is_queue_full(q)) {
        // printf("Queue is full. Cannot enqueue more items.\n");
        return;
    }
    q->data[q->rear] = item;  // Place the item in the queue at the current rear position.
    q->rear = (q->rear + 1) % QUEUE_SIZE; // Move rear to the next position.
    q->count++;
}

// Dequeue an item from the queue
struct client_info dequeue(struct queue *q) {
    if (is_queue_empty(q)) {
        // printf("Queue is empty!\n");
        struct client_info empty_client_info = { .pid = -1 };
        return empty_client_info;
    }
    struct client_info item = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE; // Move front to the next position.
    q->count--;
    return item;
}

// Get the size of the queue
int get_queue_size(struct queue *q) {
    return q->count;
}

// Print all elements in the queue
void print_queue(struct queue *q) {
    if (is_queue_empty(q)) {
        // printf("Queue is empty\n");
        return;
    }
    int i = q->front;
    for (int j = 0; j < q->count; j++) {
        // Print client PID, CWD, and wait time
        printf("Client PID: %d\n", q->data[i].pid);
        printf("Client mode: %d\n", q->data[i].mode);
        i = (i + 1) % QUEUE_SIZE;
    }
}

// Check if a PID is in the queue
int is_in_queue(struct queue *q, pid_t pid) {
    int i = q->front;
    for (int j = 0; j < q->count; j++) {
        if (q->data[i].pid == pid) {
            return 1;
        }
        i = (i + 1) % QUEUE_SIZE;
    }
    return 0;
}