#ifndef SYNCH_H
#define SYNCH_H

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <semaphore.h>

#define FNAME_LEN 128
#define NUM_OF_DIR_FILE 128

typedef struct {
    sem_t mutex;
    sem_t empty;
    sem_t full;

    //Shared
    sem_t list_dir_mutex;
} Semaphores;

struct file_sync {
    char fname[FNAME_LEN]; // file name
    int readerCount; // number of readers
    int writerCount; // number of writers
    sem_t readTry; // semaphore for readers trying to enter the critical region
    sem_t rMutex; // semaphore for readers mutual exclusion
    sem_t wMutex; // semaphore for writers mutual exclusion
    sem_t rsc; // semaphore for readers synchronization
};

struct dir_sync {
    struct file_sync files[NUM_OF_DIR_FILE]; // array of safe files in the directory
    Semaphores sems;
    int size; // current number of files in the directory
    int capacity; // maximum capacity of the directory
};

int enterRegionReader(struct file_sync *sfile); // function for a reader to enter the critical region

int exitRegionReader(struct file_sync *sfile); // function for a reader to exit the critical region

int enterRegionWriter(struct file_sync *sfile); // function for a writer to enter the critical region

int exitRegionWriter(struct file_sync *sfile); // function for a writer to exit the critical region

int initSafeDir(const char *server_dir, struct dir_sync *sdir); // initialize the safe directory

int closeSafeDir(struct dir_sync *sdir); // close the safe directory

struct file_sync *addSafeFile(struct dir_sync *sdir, const char *fname); // add a safe file to the directory

int initSafeFile(struct file_sync *sfile, const char *fname); // initialize a safe file

struct file_sync *getSafeFile(struct dir_sync *sdir, const char *file); // get a safe file from the directory

#endif
