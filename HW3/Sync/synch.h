/**
 * @file synch.h
 * @brief Header file for synchronization primitives and data structures.
 * 
 * This file defines the synchronization primitives and data structures used for safe file operations.
 * It includes the necessary headers and defines constants and structures.
 * 
 * @note This file is part of the synchronization module of the system programming project.
 * 
 */

#ifndef SYNCH_H
#define SYNCH_H

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <semaphore.h>

#define FNAME_LEN 128
#define NUM_OF_DIR_FILE 128

#define UPLOAD_AND_LIST_SYNC_FILE "05319346629"

/**
 * @struct Semaphores
 * @brief Structure to hold the semaphores used for synchronization.
 * 
 * This structure holds the semaphores used for synchronization in the system.
 * It includes semaphores for mutex, empty, full, and list directory mutex.
 */
typedef struct {
    sem_t mutex;            /**< Semaphore for mutual exclusion */
    sem_t empty;            /**< Semaphore for empty condition */
    sem_t full;             /**< Semaphore for full condition */
    sem_t list_dir_mutex;   /**< Semaphore for list directory mutex */
} Semaphores;

/**
 * @struct file_sync
 * @brief Structure to represent a safe file.
 * 
 * This structure represents a safe file in the directory.
 * It includes the file name, reader count, writer count, and various semaphores for synchronization.
 */
struct file_sync {
    sem_t readTry;          /**< Semaphore for readers trying to enter the critical region */
    sem_t rMutex;           /**< Semaphore for readers mutual exclusion */
    sem_t wMutex;           /**< Semaphore for writers mutual exclusion */
    sem_t lock;              /**< Semaphore for readers synchronization */
    char fname[FNAME_LEN];  /**< File name */
    int readerCount;        /**< Number of readers */
    int writerCount;        /**< Number of writers */
};

/**
 * @struct dir_sync
 * @brief Structure to represent a safe directory.
 * 
 * This structure represents a safe directory.
 * It includes an array of safe files, semaphores for synchronization, and other metadata.
 */
struct dir_sync {
    struct file_sync files[NUM_OF_DIR_FILE];   /**< Array of safe files in the directory */
    Semaphores sems;                           /**< Semaphores for synchronization */
    int size;                                  /**< Current number of files in the directory */
    int capacity;                              /**< Maximum capacity of the directory */
};

/**
 * @brief Put all files in the directory to the critical section.
 * 
 * This function puts all files in the directory to the critical section.
 * It acquires the necessary semaphores for synchronization.
 * 
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @return 0 if successful, -1 if an error occurred.
 */
int putAllFilesToCriticalSection(struct dir_sync *sdir);

/**
 * @brief Remove all files in the directory from the critical section.
 * 
 * This function removes all files in the directory from the critical section.
 * It releases the acquired semaphores for synchronization.
 * 
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @return 0 if successful, -1 if an error occurred.
 */
int removeAllFilesFromCriticalSection(struct dir_sync *sdir);

/*
    * @brief Put all files in the directory to the critical section except the specified file.
    * 
    * This function puts all files in the directory to the critical section except the specified file.
    * It acquires the necessary semaphores for synchronization.
    * 
    * @param sdir Pointer to the dir_sync structure representing the safe directory.
    * @param file The file name of the file to exclude from the critical section.
    * @return 0 if successful, -1 if an error occurred.
    
*/
int putAllFilesToCriticalSectionExcept(struct dir_sync *sdir, const char *file);

/*
* @brief Remove all files in the directory from the critical section except the specified file.
*
* This function removes all files in the directory from the critical section except the specified file.
* It releases the acquired semaphores for synchronization.
*
* @param sdir Pointer to the dir_sync structure representing the safe directory.
* @param file The file name of the file to exclude from the critical section.
* @return 0 if successful, -1 if an error occurred.
*/
int removeAllFilesFromCriticalSectionExcept(struct dir_sync *sdir, const char *file);

/**
 * @brief Function for a reader to enter the critical region.
 * 
 * This function is used by a reader to enter the critical region.
 * It acquires the necessary semaphores for synchronization.
 * 
 * @param sfile Pointer to the file_sync structure representing the safe file.
 * @return 0 if successful, -1 if an error occurred.
 */
int enterRegionReader(struct file_sync *sfile);

/**
 * @brief Function for a reader to exit the critical region.
 * 
 * This function is used by a reader to exit the critical region.
 * It releases the acquired semaphores for synchronization.
 * 
 * @param sfile Pointer to the file_sync structure representing the safe file.
 * @return 0 if successful, -1 if an error occurred.
 */
int exitRegionReader(struct file_sync *sfile);

/**
 * @brief Function for a writer to enter the critical region.
 * 
 * This function is used by a writer to enter the critical region.
 * It acquires the necessary semaphores for synchronization.
 * 
 * @param sfile Pointer to the file_sync structure representing the safe file.
 * @return 0 if successful, -1 if an error occurred.
 */
int enterRegionWriter(struct file_sync *sfile);

/**
 * @brief Function for a writer to exit the critical region.
 * 
 * This function is used by a writer to exit the critical region.
 * It releases the acquired semaphores for synchronization.
 * 
 * @param sfile Pointer to the file_sync structure representing the safe file.
 * @return 0 if successful, -1 if an error occurred.
 */
int exitRegionWriter(struct file_sync *sfile);

/**
 * @brief Initialize the safe directory.
 * 
 * This function initializes the safe directory.
 * It creates and initializes the necessary semaphores and sets the initial values for metadata.
 * 
 * @param server_dir The directory path for the safe directory.
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @param max_clients The maximum number of clients that can access the directory.
 * @return 0 if successful, -1 if an error occurred.
 */
int initSafeDir(const char *server_dir, struct dir_sync *sdir, int max_clients);

/**
 * @brief Close the safe directory.
 * 
 * This function closes the safe directory.
 * It releases the acquired resources and destroys the semaphores.
 * 
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @return 0 if successful, -1 if an error occurred.
 */
int closeSafeDir(struct dir_sync *sdir);

/**
 * @brief Add a safe file to the directory.
 * 
 * This function adds a safe file to the directory.
 * It creates and initializes the necessary semaphores and updates the metadata.
 * 
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @param fname The file name of the safe file.
 * @return Pointer to the file_sync structure representing the added safe file, or NULL if an error occurred.
 */
struct file_sync *addSafeFile(struct dir_sync *sdir, const char *fname);

/**
 * @brief Remove a safe file from the directory.
 * 
 * This function removes a safe file from the directory.
 * It releases the acquired resources and updates the metadata.
 * 
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @param fname The file name of the safe file to be removed.
 * @return Pointer to the file_sync structure representing the removed safe file, or NULL if the file was not found.
 */
int removeSafeFile(struct dir_sync *sdir, const char *fname);

/**
 * @brief Initialize a safe file.
 * 
 * This function initializes a safe file.
 * It creates and initializes the necessary semaphores and sets the initial values for metadata.
 * 
 * @param sfile Pointer to the file_sync structure representing the safe file.
 * @param fname The file name of the safe file.
 * @return 0 if successful, -1 if an error occurred.
 */
int initSafeFile(struct file_sync *sfile, const char *fname);

/**
 * @brief Get a safe file from the directory.
 * 
 * This function retrieves a safe file from the directory based on the file name.
 * 
 * @param sdir Pointer to the dir_sync structure representing the safe directory.
 * @param file The file name of the safe file to retrieve.
 * @return Pointer to the file_sync structure representing the retrieved safe file, or NULL if the file was not found.
 */
struct file_sync *getSafeFile(struct dir_sync *sdir, const char *file);

#endif

