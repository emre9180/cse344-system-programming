#include "common.h"
#include <fcntl.h>

// Function to create a named pipe (FIFO)
void create_named_pipe(const char* server_fifo) {
    const char* fifo_path = server_fifo;
    if (access(fifo_path, F_OK) == -1) {
        if (mkfifo(fifo_path, S_IWUSR | S_IRUSR) == -1) {
            perror("Error creating fifo");
            exit(1);
        }
    }
}