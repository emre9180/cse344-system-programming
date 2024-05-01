#include "synch.h"
#include <stdlib.h>
#include <string.h>
#include "../Util/common.h"
#include "../Server/server_util.h"

// Put all files in the directory into the critical section for reading
int putAllFilesToCriticalSection(struct dir_sync *sdir)
{
    for (int i = 0; i < sdir->size; i++)
    {
        enterRegionReader(&sdir->files[i]);
    }
    return 0;
}

// Put all files in the directory into the critical section for reading, except for a specific file
int putAllFilesToCriticalSectionExcept(struct dir_sync *sdir, const char *file)
{
    for (int i = 0; i < sdir->size; i++)
    {
        if (strcmp(sdir->files[i].fname, file) != 0)
        {
            enterRegionReader(&sdir->files[i]);
        }
    }
    return 0;
}

// Remove all files from the critical section for reading
int removeAllFilesFromCriticalSection(struct dir_sync *sdir)
{
    for (int i = 0; i < sdir->size; i++)
    {
        exitRegionReader(&sdir->files[i]);
    }
    return 0;
}

// Remove all files from the critical section for reading, except for a specific file
int removeAllFilesFromCriticalSectionExcept(struct dir_sync *sdir, const char *file)
{
    for (int i = 0; i < sdir->size; i++)
    {
        if (strcmp(sdir->files[i].fname, file) != 0)
        {
            exitRegionReader(&sdir->files[i]);
        }
    }
    return 0;
}

// Enter the reading region
int enterRegionReader(struct file_sync *sfile) {
    sem_wait(&sfile->readTry);
    sem_wait(&sfile->rMutex);
    sfile->readerCount++;
    if (sfile->readerCount == 1) {
        sem_wait(&sfile->lock);
    }
    sem_post(&sfile->rMutex);
    sem_post(&sfile->readTry);
    return 0;
}

// Exit the reading region
int exitRegionReader(struct file_sync *sfile) {
    fflush(stdout);
    sem_wait(&sfile->rMutex);
    sfile->readerCount--;
    if (sfile->readerCount == 0) {
        sem_post(&sfile->lock);
    }
    sem_post(&sfile->rMutex);
    return 0;
}

// Enter the writing region
int enterRegionWriter(struct file_sync *sfile) {
    fflush(stdout);
    sem_wait(&sfile->wMutex);
    sfile->writerCount++;
    if (sfile->writerCount == 1) {
        sem_wait(&sfile->readTry);
    }
    sem_post(&sfile->wMutex);
    sem_wait(&sfile->lock);
    return 0;
}

// Exit the writing region
int exitRegionWriter(struct file_sync *sfile) {
    fflush(stdout);
    sem_post(&sfile->lock);
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
    sem_init(&sfile->readTry, 1, 1);
    sem_init(&sfile->rMutex, 1, 1);
    sem_init(&sfile->wMutex, 1, 1);
    sem_init(&sfile->lock, 1, 1);
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
int initSafeDir(const char *server_dir, struct dir_sync *sdir, int max_clients) {
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
    addSafeFile(sdir, UPLOAD_AND_LIST_SYNC_FILE);

    // Initialize the semaphores
    if (sem_init(&sdir->sems.mutex, 1, 0) != 0) {  // mutex initialized to 1
        perror("sem_init (mutex)");
    }
    if (sem_init(&sdir->sems.empty, 1, max_clients) != 0) {  // assume 10 empty slots
        perror("sem_init (empty)");
    }
    if (sem_init(&sdir->sems.full, 1, 1) != 0) {  // initially, 0 full slots
        perror("sem_init (full)");
    }

    if (sem_init(&sdir->sems.list_dir_mutex, 1, 1) != 0) {  // initially, 0 full slots
        perror("sem_init (full)");
    }
    //Close the directory
    closedir(dir);
    
    return 0;

    
}

// Close the safe directory
int closeSafeDir(struct dir_sync *sdir) {
    for (int i = 0; i < sdir->size; i++) {
        sem_destroy(&sdir->files[i].readTry);
        sem_destroy(&sdir->files[i].rMutex);
        sem_destroy(&sdir->files[i].wMutex);
        sem_destroy(&sdir->files[i].lock);
    }
    sem_destroy(&sdir->sems.mutex);
    sem_destroy(&sdir->sems.empty);
    sem_destroy(&sdir->sems.full);

    // Unlink the semaphores

    sdir->size = 0;
    return 0;
}

// Remove a safe file from the directory
int removeSafeFile(struct dir_sync *sdir, const char *fname) {
    for (int i = 0; i < sdir->size; i++) {
        if (strcmp(sdir->files[i].fname, fname) == 0) {
            for (int j = i; j < sdir->size - 1; j++) {
                sdir->files[j] = sdir->files[j + 1];
            }
            sdir->size--;
            return 0;
        }
    }
    return -1;
}
