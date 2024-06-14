#include "../../include/Server/linked_list.h"

void initialize_linked_list(LinkedList *list) {
    list->head = NULL;
}


void add_order(LinkedList *list, Order order) {
    ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new order node");
        exit(EXIT_FAILURE);
    }
    new_node->order = order;
    new_node->next = list->head;
    list->head = new_node;
    list->size++;
}

int remove_order(LinkedList *list, int order_id) {
    ListNode *current = list->head;
    ListNode *prev = NULL;

    while (current != NULL) {
        if (current->order.order_id == order_id) {
            if (prev == NULL) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            list->size--;
            return 1; // Order removed successfully
        }
        prev = current;
        current = current->next;
    }
    return 0; // Order not found
}

Order *find_order(LinkedList *list, int order_id) {
    ListNode *current = list->head;
    while (current != NULL) {
        if (current->order.order_id == order_id) {
            return &current->order;
        }
        current = current->next;
    }
    return NULL; // Order not found
}

void print_orders(LinkedList *list) {
    ListNode *current = list->head;
    while (current != NULL) {
        Order order = current->order;
        printf("Order ID: %d, Customer ID: %d, Preparation Time: %.2f, Cooking Time: %.2f, Delivery Time: %d, Cancelled: %d, Cook ID: %d, Delivery Person ID: %d, Coordinates: (%d, %d)\n",
               order.order_id, order.customer_id, order.preparation_time, order.cooking_time, order.delivery_time, order.is_cancelled, order.cook_id, order.delivery_person_id, order.x, order.y);
        current = current->next;
    }
}
