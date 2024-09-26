#include "cmsis_os.h"
#include <stdlib.h>
#include "semaphore_interface.h"

// Abstract semaphore type definition
struct semaphore_t {
	osSemaphoreId_t sem_id; // CMSIS-RTOS v2 semaphore ID
};

// Semaphore creation function
semaphore_t* semaphore_create(void) {
	semaphore_t* sem = (semaphore_t*)malloc(sizeof(semaphore_t));
	if (sem != NULL) {
		sem->sem_id = osSemaphoreNew(1, 1, NULL); // Create a binary semaphore (initial count 1)
		if (sem->sem_id == NULL) {
			free(sem); // Free memory if semaphore creation fails
			return NULL;
		}
	}
	return sem;
}

// Take semaphore (block until available)
void semaphore_take(semaphore_t* sem) {
	if (sem != NULL && sem->sem_id != NULL) {
		osSemaphoreAcquire(sem->sem_id, osWaitForever); // Block indefinitely until semaphore is available
	}
}

// Release semaphore
void semaphore_release(semaphore_t* sem) {
	if (sem != NULL && sem->sem_id != NULL) {
		osSemaphoreRelease(sem->sem_id); // Release semaphore
	}
}

// Destroy semaphore
void semaphore_destroy(semaphore_t* sem) {
	if (sem != NULL) {
		if (sem->sem_id != NULL) {
			osSemaphoreDelete(sem->sem_id); // Delete the semaphore from the OS
		}
		free(sem); // Free the allocated memory
	}
}
