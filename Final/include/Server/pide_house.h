#ifndef PIDE_HOUSE_H
#define PIDE_HOUSE_H

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <math.h>
#include "../../include/Server/linked_list.h"
// Define constants
#define MAX_COOKS 10
#define MAX_DELIVERY_PERSONS 10
#define OVEN_CAPACITY 6
#define DELIVERY_CAPACITY 3
#define MAX_ORDERS 100
#define MOTOR_NUMBER 3
#define BAG_NUMBER 3
#define OVEN_APPARATUS_NUMBER 3


// Node structure for the queue
typedef struct QueueNode {
    Order* order;
    struct QueueNode* next;
} QueueNode;

// Queue structure
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} OrderQueue;

// Structure to represent a cook
typedef struct {
    int cook_id;
    int total_deliveries;
    double total_earnings;
    int order_id;
    int busy;
    OrderQueue *order_queue;
    pthread_mutex_t cookMutex;
    pthread_cond_t cookCond;
} Cook;

// Structure to represent a delivery person
typedef struct {
    int delivery_person_id;
    int total_deliveries;
    double total_earnings;
    int order_id;
    OrderQueue *order_bag;
    int busy;
    int current_delivery_count; // Add this to track the number of orders in the bag
} DeliveryPerson;


// Global variables
extern Cook cooks[MAX_COOKS];
extern DeliveryPerson delivery_persons[MAX_DELIVERY_PERSONS];
extern LinkedList orders;
extern int num_cooks;
extern int num_delivery_persons;
extern int order_count;
extern pthread_t *cook_threads;
extern pthread_t *delivery_threads;
extern pthread_t manager_thread;

extern volatile sig_atomic_t shutdown_flag;
extern volatile sig_atomic_t cancel_order_flag;

// Mutex for protecting the shutdown flag
extern pthread_mutex_t shutdown_mutex;
extern int total_prepared_orders;
extern int total_delivered_orders;

extern pthread_mutex_t order_mutex;
extern pthread_mutex_t delivery_mutex;
extern pthread_mutex_t cook_mutex;
extern pthread_mutex_t oven_mutex;
extern pthread_mutex_t delivery_bag_mutex;
extern pthread_mutex_t motor_mutex;
extern pthread_mutex_t apparatus_mutex;
extern pthread_mutex_t oven_putting_opening_mutex;
extern pthread_mutex_t oven_removing_opening_mutex;

extern pthread_cond_t order_cond;
extern pthread_cond_t cooked_cond;
extern pthread_cond_t oven_cond;
extern pthread_cond_t delivery_cond;
extern pthread_cond_t delivery_bag_cond;
extern pthread_cond_t motor_cond;
extern pthread_cond_t apparatus_cond;
extern pthread_cond_t oven_putting_opening_cond;
extern pthread_cond_t oven_removing_opening_cond;

extern int speed;

// Function prototypes
void initialize_system(int n_cooks, int m_delivery_persons, int speed);
void* cook_function(void *arg);
void* delivery_function(void *arg);
void* manager_function();
void place_order(Order *order);
void cancel_order(int order_id);
void handle_order_completion(Order *order);
void handle_delivery_completion(DeliveryPerson *delivery_person, Order *order);
void evaluate_performance();
void initialize_queue(OrderQueue* queue);
void enqueue(OrderQueue* queue, Order *order);
Order* dequeue(OrderQueue* queue);
int countRemainingOrders();
void printStatistics();
void writeLog(char *message);
#endif // PIDE_HOUSE_H
