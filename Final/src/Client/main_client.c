
#include "../../include/Client/client.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <num_clients> <p> <q>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    num_clients = atoi(argv[2]);
    int p = atoi(argv[3]);
    int q = atoi(argv[4]);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    initialize_clients(num_clients, server_ip, p, q);

    pthread_t *threads = (pthread_t *)malloc(num_clients * sizeof(pthread_t));
    for (int i = 0; i < num_clients; i++) {
        pthread_create(&threads[i], NULL, client_function, (void *)&clients[i]);
    }

    for (int i = 0; i < num_clients; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    cleanup();
    printf("All customers served\n");

    return 0;
}