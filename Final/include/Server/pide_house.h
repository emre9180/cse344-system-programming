#ifndef PIDE_HOUSE_H
#define PIDE_HOUSE_H

#include <pthread.h>
#include <semaphore.h>

// Define constants
#define MAX_COOKS 10
#define MAX_DELIVERY_PERSONS 10
#define OVEN_CAPACITY 6
#define DELIVERY_CAPACITY 3
#define MAX_ORDERS 100
#define MOTOR_NUMBER 3
#define BAG_NUMBER 3
#define OVEN_APPARATUS_NUMBER 3

// Structure to represent an order
typedef struct {
    int order_id;
    int customer_id;
    double preparation_time;
    double cooking_time;
    int delivery_time;
    int is_cancelled;
    int cook_id; // ID of the cook who prepared the order
    int delivery_person_id; // ID of the delivery person who delivered the order
    int x; // x-coordinate of the customer
    int y; // y-coordinate of the customer
    int taken; // 1 if the order is taken by a cook, 0 otherwise
    int ready; // 1 if the order is ready for delivery, 0 otherwise
} Order;

// Structure to represent a cook
typedef struct {
    int cook_id;
    int total_deliveries;
    double total_earnings;
    int order_id;
} Cook;

// Structure to represent a delivery person
typedef struct {
    int delivery_person_id;
    int total_deliveries;
    double total_earnings;
    int order_id;
} DeliveryPerson;

// Global variables
extern Cook cooks[MAX_COOKS];
extern DeliveryPerson delivery_persons[MAX_DELIVERY_PERSONS];
extern sem_t cook_sem;
extern sem_t delivery_sem;
extern sem_t oven_apparatus_sem;
extern sem_t oven_capacity_sem;
extern sem_t oven_putting_opening_sem;
extern sem_t oven_removing_opening_sem;
extern pthread_mutex_t order_mutex;
extern pthread_cond_t order_cond;
extern pthread_cond_t cooked_cond;
extern Order orders[MAX_ORDERS];
extern int num_cooks;
extern int num_delivery_persons;
extern int order_count;
extern pthread_t *cook_threads;
extern pthread_t *delivery_threads;
extern pthread_t manager_thread;

// Function prototypes
void initialize_system(int n_cooks, int m_delivery_persons);
void* cook_function(void *arg);
void* delivery_function(void *arg);
void* manager_function(void *arg);
void place_order(Order *order);
void cancel_order(int order_id);
void handle_order_completion(Order *order);
void handle_delivery_completion(DeliveryPerson *delivery_person, Order *order);
void evaluate_performance();

#endif // PIDE_HOUSE_H
