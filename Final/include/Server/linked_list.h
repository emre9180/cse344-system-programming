#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>

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
    int socket_fd;
} Order;
// Structure to represent a node in the linked list
typedef struct ListNode {
    Order order;
    struct ListNode *next;
} ListNode;

// Structure to represent the linked list
typedef struct {
    ListNode *head;
    int size;
} LinkedList;

// Function to initialize the linked list
void initialize_linked_list(LinkedList *list);

// Function to add an order to the linked list
void add_order(LinkedList *list, Order order);

// Function to remove an order from the linked list by order ID
int remove_order(LinkedList *list, int order_id);

// Function to find an order in the linked list by order ID
Order* find_order(LinkedList *list, int order_id);

// Function to print all orders in the linked list
void print_orders(LinkedList *list);

// Function to free the entire linked list
void free_list(LinkedList *list);

#endif // LINKED_LIST_H
