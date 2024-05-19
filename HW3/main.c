/**
 * @file main.c
 * @brief This file contains the implementation of a car parking system using threads and semaphores.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * Macros
*/

#define MAX_AUTOMOBILE_SPOTS 8 // Maximum number of slots for automobiles in autopark's PERMANENT parking lot.
#define MAX_PICKUP_SPOTS 4 // Maximum number of slots for pickups in autopark's PERMANENT parking lot.

#define MAX_AUTOMOBILE_SPOTS_TEMP 1 // Maximum number of slots for automobiles in autopark's TEMPORARY parking lot.
#define MAX_PICKUP_SPOTS_TEMP 1 // Maximum number of slots for pickups in autopark's TEMPORARY parking lot.

#define TOTAL_CAR_OWNER 45 // Total number of car owners.
#define TOTAL_CAR_ATTENDANT 2 // Total number of car attendants.

/**
 * Macros
*/


/**
 * Global Variables
*/

sem_t newPickup; // Semaphore for the number of AVALIABLE slots for PICKUPS in autopark.
sem_t inChargeforPickup; // Semaphore for the number of PICKUPS in temporary parking lot.
sem_t newAutomobile; // Semaphore for the number of AVALIABLE slots for AUTOMOBILES in autopark.
sem_t inChargeforAutomobile; // Semaphore for the number of AUTOMOBILES in temporary parking lot.

sem_t waiting; // Semaphore for the critical section.

sem_t newPickUpBusy; // Semaphore for the number of BUSY slots for PICKUPS in autopark.
sem_t newAutomobileBusy; // Semaphore for the number of BUSY slots for AUTOMOBILES in autopark.
sem_t mutex; // Mutex for the critical section for removing vehicles from autopark.

int mFree_automobile_in_temp = MAX_AUTOMOBILE_SPOTS_TEMP; // Number of free slots in temporary parking lot for automobiles.
int mFree_pickup_in_temp = MAX_PICKUP_SPOTS_TEMP; // Number of free slots in temporary parking lot for pickups.

int mAutomobile_in_system = 0; // Number of automobiles in the PERMANENT autopark for automobiles.
int mPickup_in_system = 0; // Number of pickups in the PERMANENT autopark for pickups.

/**
 * Global Variables
*/

int *finished;

/**
 * @brief Function executed by car owner threads.
 *
 * @param arg Pointer to the vehicle type (0 for Automobile, 1 for Pickup).
 * @return void*
 */
void* carOwner(void* arg)
{
    int vehicleType = *(int *)arg;
    if (vehicleType == 0) // Automobile
    {
        sem_wait(&waiting); // Enter critical section, wait if another car owner tries to park.
        printf(">> A car has arrived to autopark. Vehicle type: Automobile\n");
        if (mFree_automobile_in_temp == 0)
        { // Check if the temporary parking lot is full for automobiles.
            printf(">> The temporary parking lot's capacity is %d and it is currently full! The current slots in temporary lot is: %d/%d\n", MAX_AUTOMOBILE_SPOTS_TEMP, MAX_AUTOMOBILE_SPOTS_TEMP - mFree_automobile_in_temp, MAX_AUTOMOBILE_SPOTS_TEMP);
            printf(">> The automobile is leaving since there is no slot in the temporary parking lot for automobiles.\n");
            sem_post(&waiting); // Exit critical section. Allow other car owners to park.
        }
        else
        {
            mFree_automobile_in_temp--;                                                 // Decrement the number of free slots in temporary parking lot for automobiles.
            int busySlotsInTemp = MAX_AUTOMOBILE_SPOTS_TEMP - mFree_automobile_in_temp; // Calculate the number of busy slots in temporary parking lot for automobiles.
            printf(">> Automobile is parked in the temporary parking lot. The current status of temporary lot is: %d/%d\n", busySlotsInTemp, MAX_AUTOMOBILE_SPOTS_TEMP);
            sem_post(&waiting);               // Exit critical section. Allow other car owners to park.
            sem_post(&inChargeforAutomobile); // Signal the car attendant that an automobile is parked to temporary parking lot.
        }
    }

    else // Pickup
    {
        sem_wait(&waiting); // Enter critical section, wait if another car owner tries to park.
        printf(">> A car has arrived to autopark. Vehicle type: Pickup\n");
        if (mFree_pickup_in_temp == 0)
        { // Check if the temporary parking lot is full for pickups.
            printf(">> The temporary parking lot's capacity for pickups is %d and it is currently full! The current slots in temporary lot for pickups is: %d/%d\n", MAX_PICKUP_SPOTS_TEMP, MAX_PICKUP_SPOTS_TEMP - mFree_pickup_in_temp, MAX_PICKUP_SPOTS_TEMP);
            printf(">> The pickup is leaving since there is no slot in the temporary parking lot for pickups.\n");
            sem_post(&waiting); // Exit critical section. Allow other car owners to park.
        }
        else
        {
            mFree_pickup_in_temp--;                                             // Decrement the number of free slots in temporary parking lot for pickups.
            int busySlotsInTemp = MAX_PICKUP_SPOTS_TEMP - mFree_pickup_in_temp; // Calculate the number of busy slots in temporary parking lot for pickups.
            printf(">> Pickup is parked in the temporary parking lot. The current status of temporary lot for pickups is: %d/%d\n", busySlotsInTemp, MAX_PICKUP_SPOTS_TEMP);
            sem_post(&inChargeforPickup); // Signal the car attendant that a pickup is parked to temporary parking lot.
            sem_post(&waiting);           // Exit critical section. Allow other car owners to park.
        }
    }
    free(arg); // Free the memory allocated for the vehicle type.
    return NULL;
}


/**
 * @brief Function executed by car attendant threads.
 *
 * @param arg Pointer to the vehicle type (0 for Automobile, 1 for Pickup).
 * @return void*
 */
void *carAttendant(void *arg)
{
    int vehicleType = *(int *)arg;
    while (1)
    {
        if (vehicleType == 0)
        {                                             // Automobile
            sem_wait(&inChargeforAutomobile);         // Wait for a signal from car owner
            sem_wait(&waiting);
            int result = sem_trywait(&newAutomobile); // Try to decrement the semaphore. Wait if autopark is full for automobiles.
            if (result == 0)
            {
                // If the decrement is successful
            }
            else
            {
                printf(">> There is no permanent slot for automobile in autopark. Waiting for a slot...\n");
                sem_post(&waiting); // Exit critical section. Allow other car owners to park.
                sem_wait(&newAutomobile); // Try to decrement the semaphore. Wait if autopark is full for automobiles.
                sem_wait(&waiting); // Enter critical section, wait if another car owner tries to park.
            }
            sem_wait(&mutex);   // Lock the mutex
            sem_post(&newAutomobileBusy); // Increment the semaphore for the number of BUSY slots for AUTOMOBILES in autopark.
            mFree_automobile_in_temp++;   // Increment the number of free slots in temporary parking lot for automobiles.
            mAutomobile_in_system++;      // Increment the number of automobiles in the autopark for automobiles.
            printf(">> Automobile is parked in autopark. Current automobile slots in autopark is: %d/%d\n", mAutomobile_in_system, MAX_AUTOMOBILE_SPOTS);
            printf(">> Temporary parking lot for automobiles is now available. The current slots in temporary lot is: %d/%d\n", MAX_AUTOMOBILE_SPOTS_TEMP - mFree_automobile_in_temp, MAX_AUTOMOBILE_SPOTS_TEMP);
        }
        else
        {                                         // Pickup
            sem_wait(&inChargeforPickup);         // Wait for a signal from car owner
            sem_wait(&waiting);                   // Enter critical section, wait if another car owner tries to park.
            int result = sem_trywait(&newPickup); // Try to decrement the semaphore. Wait if autopark is full for pickups.
            if (result == 0)
            {
                // If the decrement is successful
            }
            else
            {
                printf(">> There is no slot for pickup in autopark. Waiting for a slot...\n");
                sem_post(&waiting); // Exit critical section. Allow other car owners to park.
                sem_wait(&newPickup); // Try to decrement the semaphore. Wait if autopark is full for pickups.
                sem_wait(&waiting); // Enter critical section, wait if another car owner tries to park.
            }
            sem_wait(&mutex); // Lock the mutex
            sem_post(&newPickUpBusy);   // Increment the semaphore for the number of BUSY slots for PICKUPS in autopark.
            mFree_pickup_in_temp++;     // Increment the number of free slots in temporary parking lot for pickups.
            mPickup_in_system++;        // Increment the number of pickups in the autopark for pickups.
            printf(">> Pickup is parked in autopark. Current pickup slots in autopark is: %d/%d\n", mPickup_in_system, MAX_PICKUP_SPOTS);
            printf(">> Temporary parking lot for pickups is now available. The current slots in temporary lot for pickups is: %d/%d\n", MAX_PICKUP_SPOTS_TEMP - mFree_pickup_in_temp, MAX_PICKUP_SPOTS_TEMP);
        }
        sem_post(&mutex); // Unlock the mutex
        sem_post(&waiting); // Exit critical section. Allow other car owners to park.
    }
    return NULL;
}

/**
 * @brief Function executed by car remover threads.
 *
 * @param arg Pointer to the vehicle type (0 for Automobile, 1 for Pickup).
 * @return void*
 */
void *carRemover(void *arg)
{
    int vehicleType = *(int *)arg;
    while (1)
    {
        if (*finished)
        {
            return NULL;
        }
        int random = -1;
        if ((vehicleType == 0 && mAutomobile_in_system == MAX_AUTOMOBILE_SPOTS) || (vehicleType == 1 && mPickup_in_system == MAX_PICKUP_SPOTS))
        {
            random = 0;
        }

        if(random==0)
        {
           int vehicleType = *(int*)arg;
            
            if (vehicleType == 0) {  // Automobile
                sem_wait(&mutex); // Lock the mutex
                int result = sem_trywait(&newAutomobileBusy); // Try to decrement the semaphore. Wait if autopark is full for automobiles.
                if (result == 0) {
                    // If the decrement is successful
                } else {
                    sem_post(&mutex); // Unlock the mutex to synchronize the carAttendand thread. If a car is removing, the carAttendant thread should not be able to park a car.
                    sem_wait(&newAutomobileBusy); // Try to decrement the semaphore. Wait if there is no automobile in the autopark. 
                }

                if(result!=0)
                    sem_wait(&mutex); // Lock the mutex. If the decrement is not successful, lock the mutex to synchronize the carAttendant thread. If a car is removing, the carAttendant thread should not be able to park a car.
                   

                mAutomobile_in_system--; // Decrement the number of automobiles in the autopark for automobiles.
                sem_post(&newAutomobile); // Increment the semaphore for the number of AVALIABLE slots for AUTOMOBILES in autopark.
                printf(">> REMOVING Automobile is removed from autopark. Current automobile slots in autopark is: %d/%d\n", mAutomobile_in_system, MAX_AUTOMOBILE_SPOTS);
            
            } else {  // Pickup
                sem_wait(&mutex); // Lock the mutex
                int result = sem_trywait(&newPickUpBusy); // Try to decrement the semaphore. Wait if there is no pickup in the autopark.
                if (result == 0) {
                    // If the decrement is successful
                } else {
                    sem_post(&mutex); // Unlock the mutex to synchronize the carAttendand thread. If a car is removing, the carAttendant thread should not be able to park a car.
                    sem_wait(&newPickUpBusy); // Try to decrement the semaphore. Wait if autopark is full for pickups.
                }

                if(result!=0)
                    sem_wait(&mutex); // Lock the mutex. If the decrement is not successful, lock the mutex to synchronize the carAttendant thread. If a car is removing, the carAttendant thread should not be able to park a car.

                mPickup_in_system--; // Decrement the number of pickups in the autopark for pickups.
                sem_post(&newPickup); // Increment the semaphore for the number of AVALIABLE slots for PICKUPS in autopark.
                printf(">> REMOVING Pickup is removed from autopark. Current pickup slots in autopark is: %d/%d\n", mPickup_in_system, MAX_PICKUP_SPOTS);
                
            }
            sem_post(&mutex); // Unlock the mutex
        }

        
    }
    
    free(arg); // Free the memory allocated for the vehicle type.
    return NULL;

}

int main()
{
    printf("Car Parking System\n");
    printf("Automobile slots in autopark: %d\n", MAX_AUTOMOBILE_SPOTS);
    printf("Pickup slots in autopark: %d\n", MAX_PICKUP_SPOTS);
    printf("Automobile slots in temporary parking lot: %d\n", MAX_AUTOMOBILE_SPOTS_TEMP);
    printf("Pickup slots in temporary parking lot: %d\n", MAX_PICKUP_SPOTS_TEMP);
    printf("Total car owners: %d\n", TOTAL_CAR_OWNER);
    printf("\n");

    finished = malloc(sizeof(int));
    *finished = 0;

    pthread_t ownerThreads[TOTAL_CAR_OWNER], attendantThreads[2], removerThreads[2]; // Threads for car owners, attendants, and removers
    int vehicleType[TOTAL_CAR_OWNER];                                                // Vehicle types (0 for Automobile, 1 for Pickup)

    srand(time(NULL)); // Initialize random seed

    // Initialize vehicle types
    for (int i = 0; i < TOTAL_CAR_OWNER; i++)
    {
        vehicleType[i] = rand() % 2;
    }

    // Initialize semaphores
    sem_init(&newPickup, 0, 4);
    sem_init(&inChargeforPickup, 0, 0);
    sem_init(&newAutomobile, 0, 8);
    sem_init(&inChargeforAutomobile, 0, 0);
    sem_init(&newPickUpBusy, 0, 0);
    sem_init(&newAutomobileBusy, 0, 0);
    sem_init(&waiting, 0, 1);
    sem_init(&mutex, 0, 1);

    // Create attendant threads
    int automobileType = 0;
    int pickupType = 1;
    pthread_create(&attendantThreads[0], NULL, carAttendant, &automobileType);
    pthread_create(&attendantThreads[1], NULL, carAttendant, &pickupType);

    // Create remover threads
    pthread_create(&removerThreads[0], NULL, carRemover, &automobileType);
    pthread_create(&removerThreads[1], NULL, carRemover, &pickupType);

    // Create car owner threads
    for (int i = 0; i < TOTAL_CAR_OWNER; i++)
    {
        int *vehicle = malloc(sizeof(int));
        *vehicle = vehicleType[i];
        pthread_create(&ownerThreads[i], NULL, carOwner, vehicle);
    }

    // Join owner threads
    for (int i = 0; i < TOTAL_CAR_OWNER; i++)
    {
        pthread_join(ownerThreads[i], NULL);
    }
    *finished = 1;

    // Cancel and join attendant and remover threads
    for (int i = 0; i < 2; i++)
    {
        pthread_cancel(attendantThreads[i]);
        pthread_cancel(removerThreads[i]);
        pthread_join(attendantThreads[i], NULL);
        pthread_join(removerThreads[i], NULL);
    }

    // Cleanup
    sem_destroy(&newPickup);
    sem_destroy(&inChargeforPickup);
    sem_destroy(&newAutomobile);
    sem_destroy(&inChargeforAutomobile);
    sem_destroy(&waiting);
    sem_destroy(&newPickUpBusy);
    sem_destroy(&newAutomobileBusy);
    sem_destroy(&mutex);

    printf(">> System is shutting down...\n");
    free(finished);
    return 0;
}
