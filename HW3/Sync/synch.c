#include "synch.h"
#include <stdlib.h>
#include <string.h>
#include "../Util/common.h"

// Enter the reading region
int enterRegionReader(struct file_sync *sfile) {
    sem_wait(&sfile->readTry);
    sem_wait(&sfile->rMutex);
    sfile->readerCount++;
    if (sfile->readerCount == 1) {
        sem_wait(&sfile->rsc);
    }
    sem_post(&sfile->rMutex);
    sem_post(&sfile->readTry);
    return 0;
}

// Exit the reading region
int exitRegionReader(struct file_sync *sfile) {
    sem_wait(&sfile->rMutex);
    sfile->readerCount--;
    if (sfile->readerCount == 0) {
        sem_post(&sfile->rsc);
    }
    sem_post(&sfile->rMutex);
    return 0;
}

// Enter the writing region
int enterRegionWriter(struct file_sync *sfile) {
    sem_wait(&sfile->wMutex);
    sfile->writerCount++;
    if (sfile->writerCount == 1) {
        sem_wait(&sfile->readTry);
    }
    sem_post(&sfile->wMutex);
    sem_wait(&sfile->rsc);
    return 0;
}

// Exit the writing region
int exitRegionWriter(struct file_sync *sfile) {
    sem_post(&sfile->rsc);
    sem_wait(&sfile->wMutex);
    sfile->writerCount--;
    if (sfile->writerCount == 0) {
        sem_post(&sfile->readTry);
    }
    sem_post(&sfile->wMutex);
    return 0;
}

// Initialize a safe file
int initSafeFile(struct file_sync *sfile, const char *fname) {
    strncpy(sfile->fname, fname, FNAME_LEN);
    sfile->readerCount = 0;
    sfile->writerCount = 0;
    sem_init(&sfile->readTry, 0, 1);
    sem_init(&sfile->rMutex, 0, 1);
    sem_init(&sfile->wMutex, 0, 1);
    sem_init(&sfile->rsc, 0, 1);
    return 0;
}

// Add a safe file to the directory
struct file_sync *addSafeFile(struct dir_sync *sdir, const char *fname) {
    if (sdir->size >= sdir->capacity) return NULL;
    struct file_sync *file = &sdir->files[sdir->size];
    initSafeFile(file, fname);
    sdir->size++;
    return file;
}

// Get a safe file from the directory
struct file_sync *getSafeFile(struct dir_sync *sdir, const char *file) {
    for (int i = 0; i < sdir->size; i++) {
        if (strcmp(sdir->files[i].fname, file) == 0) {
            return &sdir->files[i];
        }
    }
    return NULL;
}

// Initialize the safe directory
int initSafeDir(const char *server_dir, struct dir_sync *sdir) {
    DIR *dir = opendir(server_dir);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    struct dirent *entry;
    sdir->size = 0;
    sdir->capacity = NUM_OF_DIR_FILE;
    while ((entry = readdir(dir)) != NULL && sdir->size < sdir->capacity) {
        addSafeFile(sdir, entry->d_name);
    }
    addSafeFile(sdir, "05319346629");

    // Initialize the semaphores
    if (sem_init(&sdir->sems.mutex, 1, 0) != 0) {  // mutex initialized to 1
        perror("sem_init (mutex)");
    }
    if (sem_init(&sdir->sems.empty, 1, 1) != 0) {  // assume 10 empty slots
        perror("sem_init (empty)");
    }
    if (sem_init(&sdir->sems.full, 1, 1) != 0) {  // initially, 0 full slots
        perror("sem_init (full)");
    }

    if (sem_init(&sdir->sems.list_dir_mutex, 1, 1) != 0) {  // initially, 0 full slots
        perror("sem_init (full)");
    }
    return 0;
}

// Close the safe directory
int closeSafeDir(struct dir_sync *sdir) {
    for (int i = 0; i < sdir->size; i++) {
        sem_destroy(&sdir->files[i].readTry);
        sem_destroy(&sdir->files[i].rMutex);
        sem_destroy(&sdir->files[i].wMutex);
        sem_destroy(&sdir->files[i].rsc);
    }
    sem_destroy(&sdir->sems.mutex);
    sem_destroy(&sdir->sems.empty);
    sem_destroy(&sdir->sems.full);

    //unlink the semaphores
    
    sdir->size = 0;
    return 0;
}
