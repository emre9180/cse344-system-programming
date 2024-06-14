#include "../../include/Server/pide_house.h"
#include "../../include/Server/server_connection.h"
#include "../../include/Server/matrix.h"

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

volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t cancel_order_flag = 0;

// Mutex for protecting the shutdown flag
pthread_mutex_t shutdown_mutex = PTHREAD_MUTEX_INITIALIZER;

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
int total_delivered_orders = 0;

Order orders[MAX_ORDERS];
int order_count = 0;
pthread_t *cook_threads;
pthread_t *delivery_threads;
pthread_t manager_thread;

int speed;

void wakeup()
{
}

void initialize_queue(OrderQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        perror("Failed to initialize queue mutex");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&queue->cond, NULL) != 0)
    {
        perror("Failed to initialize queue condition variable");
        exit(EXIT_FAILURE);
    }
}

// Initialize the system
void initialize_system(int n_cooks, int m_delivery_persons, int speed_)
{
    num_cooks = n_cooks;
    num_delivery_persons = m_delivery_persons;
    speed = speed_;
    avaliable_oven = OVEN_CAPACITY;
    avaliable_motor = MOTOR_NUMBER;
    available_cooks = n_cooks;
    available_delivery_persons = m_delivery_persons;
    avaliable_apparatus = OVEN_APPARATUS_NUMBER;
    prepared_order = 0;

    cook_threads = (pthread_t *)malloc(n_cooks * sizeof(pthread_t));
    delivery_threads = (pthread_t *)malloc(m_delivery_persons * sizeof(pthread_t));

    // Initialize cooks and start cook threads
    for (int i = 0; i < n_cooks; i++)
    {
        cooks[i].cook_id = i;
        cooks[i].total_deliveries = 0;
        cooks[i].total_earnings = 0.0;
        cooks[i].order_id = -1;
        cooks[i].busy = 0;
        cooks[i].order_queue = (OrderQueue *)malloc(sizeof(OrderQueue));
        if (cooks[i].order_queue == NULL)
        {
            perror("Failed to allocate memory for order queue");
            exit(EXIT_FAILURE);
        }
        initialize_queue(cooks[i].order_queue);
        pthread_mutex_init(&cooks[i].cookMutex, NULL);
        pthread_cond_init(&cooks[i].cookCond, NULL);

        pthread_create(&cook_threads[i], NULL, cook_function, (void *)&cooks[i]);
    }

    // Initialize delivery persons and start delivery threads
    for (int i = 0; i < m_delivery_persons; i++)
    {
        delivery_persons[i].delivery_person_id = i;
        delivery_persons[i].total_deliveries = 0;
        delivery_persons[i].total_earnings = 0.0;
        delivery_persons[i].order_id = -1;
        delivery_persons[i].busy = 0;
        delivery_persons[i].current_delivery_count = 0;
        delivery_persons[i].order_bag = (OrderQueue *)malloc(sizeof(OrderQueue));
        if (delivery_persons[i].order_bag == NULL)
        {
            perror("Failed to allocate memory for order bag");
            exit(EXIT_FAILURE);
        }
        initialize_queue(delivery_persons[i].order_bag);
        pthread_mutex_init(&delivery_persons[i].order_bag->mutex, NULL);
        pthread_cond_init(&delivery_persons[i].order_bag->cond, NULL);
        pthread_create(&delivery_threads[i], NULL, delivery_function, (void *)&delivery_persons[i]);
    }

    pthread_create(&manager_thread, NULL, manager_function, NULL);
}

Cook *findAppropriateCook()
{
    Cook *cook = NULL;
    double min_earnings = INFINITY;

    for (int i = 0; i < num_cooks; i++)
    {
        if (cooks[i].busy == 0)
        {
            cook = &cooks[i];
        }
    }

    return cook;
}

Order *findAppropriateOrder()
{
    Order *order = NULL;

    for (int i = 0; i < order_count; i++)
    {
        if (orders[i].taken == 0)
        {
            order = &orders[i];
        }
    }

    return order;
}

DeliveryPerson *findAppropriateDeliveryPerson()
{
    DeliveryPerson *delivery_person = NULL;

    for (int i = 0; i < num_delivery_persons; i++)
    {
        if (delivery_persons[i].current_delivery_count < DELIVERY_CAPACITY)
        {
            delivery_person = &delivery_persons[i];
            break; // Find the first available delivery person
        }
    }

    return delivery_person;
}

void *manager_function(void *arg)
{
    while (1)
    {
        if (shutdown_flag)
        {
            wakeup();
            break; // Exit the loop
        }
        // printf("prepared order and order ount: %d %d\n", prepared_order, order_count);
        if(countRemainingOrders() < 3 && prepared_order != order_count){
            // Send signal to all delivery man conds
            for (int i = 0; i < num_delivery_persons; i++)
            {
                pthread_cond_signal(&delivery_persons[i].order_bag->cond);
            }
        }

        else if(total_delivered_orders==order_count && order_count>0){
            printStatistics();
            break;
        }

      

        pthread_mutex_lock(&order_mutex);

        // Wait if there are no new orders
        if (order_count == 0)
        {
            pthread_cond_wait(&order_cond, &order_mutex);
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&order_mutex);
                break; // Exit the loop
            }
        }

        Order *order = findAppropriateOrder();
        if (order != NULL)
        {
            Cook *cook = findAppropriateCook();
            if (cook != NULL)
            {
                available_cooks--;
                order->taken = 1;
                enqueue(cook->order_queue, order);
                cook->busy = 1;
                cook->order_id = order->order_id;
                pthread_cond_signal(&cook->cookCond);
                send_response(order->socket_fd, "Order is sent to cook.\n");
                printf("Order %d assigned to cook %d.\n", order->order_id, cook->cook_id);
            }
        }
        pthread_mutex_unlock(&order_mutex);

        // Check for completed orders and assign to delivery personnel
        for (int i = 0; i < order_count; i++)
        {
            order = &orders[i];
            if (order->ready == 1 && order->taken == 1 && order->delivery_person_id == -1)
            {
                
                fflush(stdout);
                DeliveryPerson *delivery_person = findAppropriateDeliveryPerson();
                if (delivery_person != NULL)
                {
                    order->delivery_person_id = delivery_person->delivery_person_id;
                    enqueue(delivery_person->order_bag, order);
                    delivery_person->current_delivery_count++;
                    if (delivery_person->current_delivery_count == DELIVERY_CAPACITY)
                    {
                        delivery_person->busy = 1;
                        pthread_cond_signal(&delivery_person->order_bag->cond);
                        send_response(order->socket_fd, "Order is sent to delivery man.\n");
                        printf("\n\n\n\n\n\nDelivery person %d is ready to deliver orders.\n", delivery_person->delivery_person_id);
                        printf("Reamining orders: %d\n", countRemainingOrders());
                        fflush(stdout);
                    }
                    else
                    {
                        printf("Order %d added to delivery person %d's bag.\n", order->order_id, delivery_person->delivery_person_id);
                        fflush(stdout);
                    }
                }
            }
        }
    }
    pthread_exit(NULL);
}

void *cook_function(void *arg)
{
    Cook *cook = (Cook *)arg;

    while (1)
    {
        if (shutdown_flag)
        {
            wakeup();
            pthread_mutex_unlock(&cook->cookMutex);
            printf("Cook %d exiting...\n", cook->cook_id);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&cook->cookMutex);
        // Wait for an order to be enqueued
        while (cook->order_queue->front == NULL)
        {
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&cook->cookMutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&cook->cookCond, &cook->cookMutex);
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&cook->cookMutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
        }

        if (shutdown_flag)
        {
            wakeup();
            pthread_mutex_unlock(&cook->cookMutex);
            printf("Cook %d exiting...\n", cook->cook_id);
            pthread_exit(NULL);
        }

        Order *order = dequeue(cook->order_queue);
        pthread_mutex_unlock(&cook->cookMutex);

        if (order == NULL)
        {
            continue; // Handle spurious wakeups
        }

        printf("Cook %d is preparing order %d\n", cook->cook_id, order->order_id);
        double time = getTime();
        usleep(order->preparation_time * 1000); // Convert to microseconds

        // Acquire an apparatus
        pthread_mutex_lock(&apparatus_mutex);
        printf("Cook %d is taking an apparatus\n", cook->cook_id);
        while (avaliable_apparatus <= 0)
        {
            printf("Cook %d is waiting for an apparatus\n", cook->cook_id);
            pthread_cond_wait(&apparatus_cond, &apparatus_mutex);
            if (shutdown_flag)
            {
                pthread_mutex_unlock(&apparatus_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
        }
        avaliable_apparatus--;
        pthread_mutex_unlock(&apparatus_mutex);

        // Acquire take opening
        pthread_mutex_lock(&oven_putting_opening_mutex);
        while (put_cook_opening <= 0)
        {
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&oven_putting_opening_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
            printf("Cook %d is waiting for the oven opening\n", cook->cook_id);
            pthread_cond_wait(&oven_putting_opening_cond, &oven_putting_opening_mutex);
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&oven_putting_opening_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
        }
        if (shutdown_flag)
        {
            wakeup();
            pthread_mutex_unlock(&oven_putting_opening_mutex);
            printf("Cook %d exiting...\n", cook->cook_id);
            pthread_exit(NULL);
        }
        printf("Cook %d is taking the oven opening\n", cook->cook_id);
        put_cook_opening--;
        pthread_mutex_unlock(&oven_putting_opening_mutex);

        // Check for oven space
        pthread_mutex_lock(&oven_mutex);
        while (avaliable_oven <= 0)
        {
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&oven_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }

            printf("Cook %d is waiting for oven space\n", cook->cook_id);
            pthread_mutex_lock(&apparatus_mutex);
            avaliable_apparatus++; // Release the apparatus
            printf("Cook %d is putting an apparatus back\n", cook->cook_id);
            pthread_cond_signal(&oven_cond); // Notify that the apparatus is available
            pthread_mutex_unlock(&apparatus_mutex);

            pthread_mutex_lock(&oven_putting_opening_mutex);
            put_cook_opening++; // Release the take opening
            printf("Cook %d is putting the oven opening back\n", cook->cook_id);
            pthread_cond_signal(&oven_putting_opening_cond); // Notify that the take opening is available
            pthread_mutex_unlock(&oven_putting_opening_mutex);

            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&oven_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }

            pthread_cond_wait(&oven_cond, &oven_mutex); // Wait for space in the oven
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&oven_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
        }

        if (shutdown_flag)
        {
            wakeup();
            pthread_mutex_unlock(&oven_mutex);
            printf("Cook %d exiting...\n", cook->cook_id);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&oven_mutex);

        pthread_mutex_lock(&apparatus_mutex);
        printf("Cook %d is putting an apparatus back\n", cook->cook_id);
        avaliable_apparatus++;           // Re-acquire the apparatus
        pthread_cond_signal(&oven_cond); // Notify that the oven space is available
        pthread_mutex_unlock(&apparatus_mutex);

        pthread_mutex_lock(&oven_putting_opening_mutex);
        printf("Cook %d is putting the oven opening for putting back\n", cook->cook_id);
        put_cook_opening++;                              // Re-acquire the take opening
        pthread_cond_signal(&oven_putting_opening_cond); // Notify that the take opening is available
        pthread_mutex_unlock(&oven_putting_opening_mutex);

        pthread_mutex_lock(&oven_mutex);
        avaliable_oven--;
        pthread_mutex_unlock(&oven_mutex);

        printf("Cook %d is putting order %d in the oven\n", cook->cook_id, order->order_id);
        usleep(time/2* 1000); // Simulate cooking time

        // Acquire an apparatus again
        pthread_mutex_lock(&apparatus_mutex);
        while (avaliable_apparatus <= 0)
        {
            printf("Cook %d is waiting for an apparatus\n", cook->cook_id);
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&apparatus_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&apparatus_cond, &apparatus_mutex);
            if (shutdown_flag)
            {
                pthread_mutex_unlock(&apparatus_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
        }
        avaliable_apparatus--;
        pthread_mutex_unlock(&apparatus_mutex);

        // Acquire put opening
        pthread_mutex_lock(&oven_removing_opening_mutex);
        while (take_cook_opening <= 0)
        {
            printf("Cook %d is waiting for the oven opening for removing\n", cook->cook_id);
            pthread_cond_wait(&oven_removing_opening_cond, &oven_removing_opening_mutex);
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&oven_removing_opening_mutex);
                printf("Cook %d exiting...\n", cook->cook_id);
                pthread_exit(NULL);
            }
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
        take_cook_opening++;                              // Release the put opening
        pthread_cond_signal(&oven_removing_opening_cond); // Notify that the put opening is available
        printf("Cook %d is putting the oven opening for removal back\n", cook->cook_id);
        pthread_cond_signal(&oven_cond); // Notify that the oven space is available
        pthread_mutex_unlock(&oven_removing_opening_mutex);

        // Mark the order as completed
        pthread_mutex_lock(&order_mutex);
        order->ready = 1;
        available_cooks++; // Release the cook
        prepared_order++;
        cook->total_deliveries++;
        pthread_cond_signal(&cooked_cond); // Notify that the order is cooked
        total_prepared_orders++;
        printf("Total prepared orders: %d\n", total_prepared_orders);
        printf("Total orders: %d\n", order_count);
        cook->busy = 0;
        pthread_mutex_unlock(&order_mutex);
    }
}

void *delivery_function(void *arg)
{
    DeliveryPerson *delivery_person = (DeliveryPerson *)arg;

    while (1)
    {
        if (shutdown_flag)
        {
            wakeup();
            //printf("Delivery person %d exiting...\n", delivery_person->delivery_person_id);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(&delivery_person->order_bag->mutex);
        // Wait for orders in the bag
        while (delivery_person->order_bag->front == NULL || (delivery_person->current_delivery_count < DELIVERY_CAPACITY && countRemainingOrders()>=3))
        {
            //printf("Delivery person %d is waiting for orders\n\n\n\n\n", delivery_person->delivery_person_id);
            pthread_cond_wait(&delivery_person->order_bag->cond, &delivery_person->order_bag->mutex);
            if (shutdown_flag)
            {
                wakeup();
                pthread_mutex_unlock(&delivery_person->order_bag->mutex);
                //printf("Delivery person %d exiting...\n", delivery_person->delivery_person_id);
                pthread_exit(NULL);
            }
        }

        pthread_mutex_unlock(&delivery_person->order_bag->mutex);
        int delivery_count = 0;
        Order *orders_to_deliver[DELIVERY_CAPACITY];

        // Collect orders from the bag
        while (delivery_count < DELIVERY_CAPACITY && delivery_person->order_bag->front != NULL)
        {
            orders_to_deliver[delivery_count++] = dequeue(delivery_person->order_bag);
        }
                            printf("Uyandi ve delivery count: %d\n", delivery_person->current_delivery_count);

        delivery_person->current_delivery_count = 0; // Reset count after collecting orders

        if (delivery_count > 0)
        {
            //printf("Delivery person %d is delivering %d orders\n", delivery_person->delivery_person_id, delivery_count);
            fflush(stdout);
            double total_delivery_time = 0;
            for (int i = 0; i < delivery_count; i++)
            {
                handle_delivery_completion(delivery_person, orders_to_deliver[i]);
                 // Calculate the Euclidean distance from the Pide House (p/2, q/2) to the customer (order->x, order->y)
                double distance = sqrt(pow(orders_to_deliver[i]->x - (p / 2), 2) + pow(orders_to_deliver[i]->y - (q / 2), 2));
                double delivery_time = distance / speed; // Assume DELIVERY_SPEED is a constant speed factor

                total_delivery_time += delivery_time;

                // Sleep to simulate delivery time
                printf("Delivery person %d delivering order %d over a distance of %.2f km\n", delivery_person->delivery_person_id, orders_to_deliver[i]->order_id, distance);
                usleep(delivery_time * 1000000); // Convert seconds to microseconds for usleep
                orders_to_deliver[i]->delivery_time = delivery_time;

                delivery_person->total_earnings += 10.0; // Example earning per delivery
                printf("Order %d delivered by delivery person %d\n", orders_to_deliver[i]->order_id, delivery_person->delivery_person_id);
            }

            // Reset delivery person status
            pthread_mutex_lock(&delivery_mutex);
            delivery_person->busy = 0;

            total_delivered_orders += delivery_count;
            printf("Total delivered orders: %d\n", total_delivered_orders);
            fflush(stdout);
            pthread_mutex_unlock(&delivery_mutex);
        }
    }

    pthread_exit(NULL);
}

void place_order(Order *order)
{
    pthread_mutex_lock(&order_mutex);
    order->order_id = order_count++;
    order->cook_id = -1;
    order->delivery_person_id = -1;
    orders[order->order_id] = *order;
    pthread_cond_signal(&order_cond);
    pthread_mutex_unlock(&order_mutex);
}

void cancel_order(int order_id)
{
    pthread_mutex_lock(&order_mutex);
    if (order_id < order_count)
    {
        orders[order_id].is_cancelled = 1;
    }
    pthread_mutex_unlock(&order_mutex);
}

void handle_order_completion(Order *order)
{
    printf("Order %d is prepared by cook %d\n", order->order_id, order->cook_id);
}

void handle_delivery_completion(DeliveryPerson *delivery_person, Order *order)
{
    printf("Order %d is delivered by delivery person %d\n", order->order_id, delivery_person->delivery_person_id);
    delivery_person->total_deliveries++;
    delivery_person->total_earnings += 10.0; // Example earning per delivery
}

void evaluate_performance()
{
    pthread_mutex_lock(&order_mutex);
    DeliveryPerson *best_delivery_person = NULL;
    double max_deliveries = 0;

    for (int i = 0; i < num_delivery_persons; i++)
    {
        if (delivery_persons[i].total_deliveries > max_deliveries)
        {
            best_delivery_person = &delivery_persons[i];
            max_deliveries = delivery_persons[i].total_deliveries;
        }
    }

    if (best_delivery_person)
    {
        printf("Best delivery person: %d with %d deliveries\n", best_delivery_person->delivery_person_id, best_delivery_person->total_deliveries);
    }
    pthread_mutex_unlock(&order_mutex);
}

void enqueue(OrderQueue *queue, Order *order)
{
    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!new_node)
    {
        perror("Failed to allocate memory for queue node");
        exit(EXIT_FAILURE);
    }
    new_node->order = order;
    new_node->next = NULL;
    pthread_mutex_lock(&queue->mutex);
    if (queue->rear)
    {
        queue->rear->next = new_node;
    }
    else
    {
        queue->front = new_node;
    }
    queue->rear = new_node;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

Order *dequeue(OrderQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    while (queue->front == NULL)
    {
        pthread_cond_wait(&queue->cond, &queue->mutex);
        if (shutdown_flag)
        {
            wakeup();
            pthread_mutex_unlock(&queue->mutex);
            pthread_exit(NULL);
        }
    }

    QueueNode *temp = queue->front;
    Order *order;

    order = temp->order;
    queue->front = temp->next;
    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    free(temp);
    pthread_mutex_unlock(&queue->mutex);
    return order; // Successfully dequeued and return the order
}

int countRemainingOrders()
{
    int count = 0;
    for (int i = 0; i < order_count; i++)
    {
        if (orders[i].delivery_person_id == -1)
        {
            count++;
        }
    }
    return count;
}

void printStatistics()
{
    printf("Statistics:\n");
    printf("Cook Statistics:\n");
    fflush(stdout);
    for (int i = 0; i < num_cooks; i++)
    {
        printf("Cook %d cooked %d orders\n", cooks[i].cook_id, cooks[i].total_deliveries);
    }

    printf("Delivery Person Statistics:\n");
    for (int i = 0; i < num_delivery_persons; i++)
    {
        printf("Delivery Person %d delivered %d orders and earned %.2f TL\n", delivery_persons[i].delivery_person_id, delivery_persons[i].total_deliveries, delivery_persons[i].total_earnings);
    }

    printf("Total prepared orders: %d\n", total_prepared_orders);
    printf("Total delivered orders: %d\n", total_delivered_orders);
}
