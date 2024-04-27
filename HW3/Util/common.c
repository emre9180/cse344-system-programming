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
    //printf("A named pipe (FIFO) created successfully.\n");
    //printf("Path: %s\n", server_fifo);
}

// // Function to initialize shared memory
// Semaphores *initialize_shared_memory() {
//     int shm_fd;
//     void *shm_addr;

//     // Create the shared memory object
//     shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
//     if (shm_fd == -1) {
//         perror("shm_open");
//         exit(EXIT_FAILURE);
//     }

//     // Set the size of the shared memory object
//     if (ftruncate(shm_fd, SHM_SIZE) == -1) {
//         perror("ftruncate");
//         close(shm_fd);
//         shm_unlink(SHM_NAME);
//         exit(EXIT_FAILURE);
//     }

//     // Map the shared memory object
//     shm_addr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (shm_addr == MAP_FAILED) {
//         perror("mmap");
//         close(shm_fd);
//         shm_unlink(SHM_NAME);
//         exit(EXIT_FAILURE);
//     }

//     // Get the pointer to the structured semaphores
//     Semaphores *sems = (Semaphores *)shm_addr;

//     // Initialize the semaphores
//     if (sem_init(&sems->mutex, 1, 0) != 0) {  // mutex initialized to 1
//         perror("sem_init (mutex)");
//     }
//     if (sem_init(&sems->empty, 1, 1) != 0) {  // assume 10 empty slots
//         perror("sem_init (empty)");
//     }
//     if (sem_init(&sems->full, 1, 1) != 0) {  // initially, 0 full slots
//         perror("sem_init (full)");
//     }

//     if (sem_init(&sems->list_dir_mutex, 1, 1) != 0) {  // initially, 0 full slots
//         perror("sem_init (full)");
//     }



//     printf("Semaphores initialized in shared memory.\n");
//     return sems;
// }


//     // Open the existing shared memory object
//     *shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
//     if (*shm_fd == -1) {
//         perror("shm_open");
//         exit(EXIT_FAILURE);
//     }

//     // Map the shared memory object
//     *shm_addr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, *shm_fd, 0);
//     if (*shm_addr == MAP_FAILED) {
//         perror("mmap");
//         close(*shm_fd);
//         exit(EXIT_FAILURE);
//     }
// }