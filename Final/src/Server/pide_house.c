#include "../../include/Server/pide_house.h"
#include "../../include/Server/server_connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// Global variables
Cook cooks[MAX_COOKS];
DeliveryPerson delivery_persons[MAX_DELIVERY_PERSONS];
// sem_t cook_sem;
// sem_t motor_number;
// sem_t delivery_bag_sem;
// sem_t oven_apparatus_sem;
// sem_t oven_capacity_sem;
// sem_t oven_putting_opening_sem;
// sem_t oven_removing_opening_sem;

pthread_mutex_t order_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t delivery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cook_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t oven_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t delivery_bag_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t motor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t apparatus_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t oven_putting_opening_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t oven_removing_opening_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t order_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cooked_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t oven_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t delivery_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t delivery_bag_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t motor_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t apparatus_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t oven_putting_opening_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t oven_removing_opening_cond = PTHREAD_COND_INITIALIZER;

int available_cooks = 0;
int available_delivery_persons = 0;
int avaliable_oven = 0;
int avaliable_motor = 0;
int avaliable_apparatus = 0;
int prepared_order = 0;
int take_cook_opening = 1;
int put_cook_opening = 1;

int num_delivery_persons = 0;
int num_cooks = 0;


int total_prepared_orders = 0;

Order orders[MAX_ORDERS];
int order_count = 0;
pthread_t *cook_threads;
pthread_t *delivery_threads;
pthread_t manager_thread;

void initialize_system(int n_cooks, int m_delivery_persons) {
    num_cooks = n_cooks;
    num_delivery_persons = m_delivery_persons;

    avaliable_oven = OVEN_CAPACITY;
    avaliable_motor = MOTOR_NUMBER;
    available_cooks = n_cooks;
    available_delivery_persons = m_delivery_persons;
    avaliable_apparatus = OVEN_APPARATUS_NUMBER;
    prepared_order = 0;

    for (int i = 0; i < n_cooks; i++) {
        cooks[i].cook_id = i;
        cooks[i].total_deliveries = 0;
        cooks[i].total_earnings = 0.0;
        cooks[i].order_id = -1;
    }

    // sem_init(&cook_sem, 0, n_cooks);
    // sem_init(&motor_number, 0, 3);
    // sem_init(&oven_apparatus_sem, 0, 3);
    // sem_init(&oven_capacity_sem, 0, OVEN_CAPACITY);
    // sem_init(&oven_putting_opening_sem, 0, 1);
    // sem_init(&oven_removing_opening_sem, 0, 1);
    // sem_init(&delivery_bag_sem, 0, 3);

    cook_threads = (pthread_t *)malloc(n_cooks * sizeof(pthread_t));
    delivery_threads = (pthread_t *)malloc(m_delivery_persons * sizeof(pthread_t));

    pthread_create(&manager_thread, NULL, manager_function, NULL);
}

Cook* findAppropriateCook() {
    Cook *cook = NULL;
    double min_earnings = INFINITY;

    for (int i = 0; i < num_cooks; i++) {
        if (cooks[i].busy == 0) {
            cook = &cooks[i];
        }
    }

    return cook;
}

Order* findAppropriateOrder() {
    Order *order = NULL;

    for (int i = 0; i < order_count; i++) {
        if (orders[i].taken == 0) {
            order = &orders[i];
        }
    }

    return order;
}

DeliveryPerson* findAppropriateDeliveryPerson() {
    DeliveryPerson *delivery_person = NULL;
    double min_earnings = INFINITY;

    for (int i = 0; i < num_delivery_persons; i++) {
        if (delivery_persons[i].order_id == 0) {
            delivery_person = &delivery_persons[i];
        }
    }

    return delivery_person;
}
void* manager_function(void *arg) {
    while (prepared_order <= 50) {
        // Continuously check for new orders and assign them to cooks
        pthread_mutex_lock(&order_mutex);
        
        Order *order = findAppropriateOrder();
        if (order_count!=0 && order != NULL) {
            if (available_cooks!=0) { // Try to get a cook without blocking
                Cook *cook = findAppropriateCook();
                if (cook != NULL) {
                    cook->order_id = order->order_id;
                    order->taken = 1;
                    pthread_t cook_thread;
                    available_cooks--;
                    pthread_create(&cook_thread, NULL, cook_function, (void *)cook);
                    pthread_detach(cook_thread);
                    send_response(order->socket_fd, "Order is sent to cook.\n");

                } else {
                    // Release semaphore if no appropriate cook found
                    printf("No appropriate cook found\n");
                }
            }

            else if(available_cooks!=0 && prepared_order == 0){
                pthread_cond_wait(&cooked_cond, &order_mutex);
            }
        }
        pthread_mutex_unlock(&order_mutex);

        // Continuously check for completed orders and assign them to delivery personnel
        pthread_mutex_lock(&delivery_mutex);

        for (int i = 0; i < order_count; i++) {
            order = &orders[i];
            if (order->ready && order->taken && order->delivery_person_id == -1){
                if (available_delivery_persons!=0) { // Try to get a delivery person without blocking
                    DeliveryPerson *delivery_person = findAppropriateDeliveryPerson();
                    if (delivery_person != NULL) {
                        delivery_person->order_id = order->order_id;
                        pthread_t delivery_thread;
                        pthread_create(&delivery_thread, NULL, delivery_function, (void *)delivery_person);
                        pthread_detach(delivery_thread);

                    } else {
                        // Release semaphore if no appropriate delivery person found
                        // printf("No appropriate delivery person found\n");
                    }
                }
            }
        }
        pthread_mutex_unlock(&delivery_mutex);
    }
    pthread_exit(NULL);
}


void* cook_function(void *arg) {
    Cook *cook = (Cook *)arg;
    Order *order = orders + cook->order_id;

    printf("Cook %d is preparing order %d\n", cook->cook_id, order->order_id);
    usleep(order->preparation_time * 1000); // Simulate preparation time

    // Acquire an apparatus
    pthread_mutex_lock(&apparatus_mutex);
    printf("Cook %d is taking an apparatus\n", cook->cook_id);
    while (avaliable_apparatus <= 0) {
        printf("Cook %d is waiting for an apparatus\n", cook->cook_id);
        pthread_cond_wait(&apparatus_cond, &apparatus_mutex);
    }
    avaliable_apparatus--;
    pthread_mutex_unlock(&apparatus_mutex);

    // Acquire take opening
    pthread_mutex_lock(&oven_putting_opening_mutex);
    while (put_cook_opening <= 0) {
        printf("Cook %d is waiting for the oven opening\n", cook->cook_id);
        pthread_cond_wait(&oven_putting_opening_cond, &oven_putting_opening_mutex);
    }
    printf("Cook %d is taking the oven opening\n", cook->cook_id);
    put_cook_opening--;
    pthread_mutex_unlock(&oven_putting_opening_mutex);

    // Check for oven space
    pthread_mutex_lock(&oven_mutex);
    while (avaliable_oven <= 0) {
        printf("Cook %d is waiting for oven space\n", cook->cook_id);
        pthread_mutex_lock(&apparatus_mutex);
        avaliable_apparatus++; // Release the apparatus
        printf("Cook %d is putting an apparatus back\n", cook->cook_id);
        pthread_cond_signal(&oven_cond); // Notify that the apparatus is available
        pthread_mutex_unlock(&apparatus_mutex);

        pthread_mutex_lock(&oven_putting_opening_mutex);
        put_cook_opening++;   // Release the take opening
        printf("Cook %d is putting the oven opening back\n", cook->cook_id);
        pthread_cond_signal(&oven_putting_opening_cond); // Notify that the take opening is available
        pthread_mutex_unlock(&oven_putting_opening_mutex);

        pthread_cond_wait(&oven_cond, &oven_mutex); // Wait for space in the oven
    }

    pthread_mutex_lock(&apparatus_mutex);
    printf("Cook %d is putting an apparatus back\n", cook->cook_id);
    avaliable_apparatus++; // Re-acquire the apparatus
    pthread_cond_signal(&oven_cond); // Notify that the oven space is available
    pthread_mutex_unlock(&apparatus_mutex);

    pthread_mutex_lock(&oven_putting_opening_mutex);
    printf("Cook %d is putting the oven opening for putting back\n", cook->cook_id);
    put_cook_opening++;   // Re-acquire the take opening
    pthread_cond_signal(&oven_putting_opening_cond); // Notify that the take opening is available
    pthread_mutex_unlock(&oven_putting_opening_mutex);

    avaliable_oven--;
    pthread_mutex_unlock(&oven_mutex);




    printf("Cook %d is putting order %d in the oven\n", cook->cook_id, order->order_id);
    usleep(order->cooking_time * 1000); // Simulate cooking time

    // Acquire an apparatus again
    pthread_mutex_lock(&apparatus_mutex);
    while (avaliable_apparatus <= 0) {
        printf("Cook %d is waiting for an apparatus\n", cook->cook_id);
        pthread_cond_wait(&apparatus_cond, &apparatus_mutex);
    }
    avaliable_apparatus--;
    pthread_mutex_unlock(&apparatus_mutex);

    // Acquire put opening
    pthread_mutex_lock(&oven_removing_opening_mutex);
    while (take_cook_opening <= 0) {
        printf("Cook %d is waiting for the oven opening for removing\n", cook->cook_id);
        pthread_cond_wait(&oven_removing_opening_cond, &oven_removing_opening_mutex);
    }
    take_cook_opening--;
    pthread_mutex_unlock(&oven_removing_opening_mutex);

    // Remove from oven
    pthread_mutex_lock(&oven_mutex);
    avaliable_oven++;
    printf("Cook %d is removing order %d from the oven\n", cook->cook_id, order->order_id);
    pthread_cond_signal(&oven_cond); // Notify that the oven space is available
    printf("Cook %d finished cooking order %d\n", cook->cook_id, order->order_id);
    pthread_mutex_unlock(&oven_mutex);

    pthread_mutex_lock(&apparatus_mutex);
    avaliable_apparatus++; // Release the apparatus
    printf("Cook %d is putting an apparatus back\n", cook->cook_id);
    pthread_cond_signal(&apparatus_cond); // Notify that the apparatus is available
    pthread_mutex_unlock(&apparatus_mutex);

    pthread_mutex_lock(&oven_removing_opening_mutex);
    take_cook_opening++;    // Release the put opening
    pthread_cond_signal(&oven_removing_opening_cond); // Notify that the put opening is available
    printf("Cook %d is putting the oven opening for removal back\n", cook->cook_id);
    pthread_cond_signal(&oven_cond); // Notify that the oven space is available
    pthread_mutex_unlock(&oven_removing_opening_mutex);



    // Mark the order as completed
    pthread_mutex_lock(&order_mutex);
    order->ready = 1;
    available_cooks++; // Release the cook
    prepared_order++;
    pthread_cond_signal(&cooked_cond); // Notify that the order is cooked
    total_prepared_orders++;
    printf("Total prepared orders: %d\n", total_prepared_orders);
    printf("Total orders: %d\n", order_count);
    cook->order_id = -1;
    for(int i = 0; i < order_count; i++){
        if(orders[i].ready == 0 && total_prepared_orders>45){
            printf("order %d is not ready\n", orders[i].order_id);
        }
    }
    pthread_mutex_unlock(&order_mutex);
    pthread_exit(NULL);
}

void* delivery_function(void *arg) {
    // DeliveryPerson *delivery_person = (DeliveryPerson *)arg;
    // if (delivery_person->current_delivery_count == 0) return NULL;

    // printf("Delivery person %d is delivering orders\n", delivery_person->delivery_person_id);
    // int total_delivery_time = 0;
    // for (int i = 0; i < delivery_person->current_delivery_count; i++) {
    //     Order *order = delivery_person->orders_to_deliver[i];
    //     total_delivery_time += order->delivery_time;
    // }
    // usleep(total_delivery_time * 1000000); // Simulate delivery time

    // pthread_mutex_lock(&order_mutex);
    // for (int i = 0; i < delivery_person->current_delivery_count; i++) {
    //     Order *order = delivery_person->orders_to_deliver[i];
    //     handle_delivery_completion(delivery_person, order);
    // }
    // delivery_person->current_delivery_count = 0;
    // pthread_mutex_unlock(&order_mutex);

    // sem_post(&delivery_sem);

    pthread_exit(NULL);
}
void place_order(Order *order) {
    pthread_mutex_lock(&order_mutex);
    order->order_id = order_count++;
    order->cook_id = -1;
    order->delivery_person_id = -1;
    orders[order->order_id] = *order;
    pthread_cond_signal(&order_cond);
    pthread_mutex_unlock(&order_mutex);
}

void cancel_order(int order_id) {
    pthread_mutex_lock(&order_mutex);
    if (order_id < order_count) {
        orders[order_id].is_cancelled = 1;
    }
    pthread_mutex_unlock(&order_mutex);
}

void handle_order_completion(Order *order) {
    printf("Order %d is prepared by cook %d\n", order->order_id, order->cook_id);
}

void handle_delivery_completion(DeliveryPerson *delivery_person, Order *order) {
    printf("Order %d is delivered by delivery person %d\n", order->order_id, delivery_person->delivery_person_id);
    delivery_person->total_deliveries++;
    delivery_person->total_earnings += 10.0; // Example earning per delivery
}

void evaluate_performance() {
    pthread_mutex_lock(&order_mutex);
    DeliveryPerson *best_delivery_person = NULL;
    double max_deliveries = 0;

    for (int i = 0; i < num_delivery_persons; i++) {
        if (delivery_persons[i].total_deliveries > max_deliveries) {
            best_delivery_person = &delivery_persons[i];
            max_deliveries = delivery_persons[i].total_deliveries;
        }
    }

    if (best_delivery_person) {
        printf("Best delivery person: %d with %d deliveries\n", best_delivery_person->delivery_person_id, best_delivery_person->total_deliveries);
    }
    pthread_mutex_unlock(&order_mutex);
}
