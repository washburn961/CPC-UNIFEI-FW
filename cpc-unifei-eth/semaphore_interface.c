#include "cmsis_os.h"
#include <stdlib.h>
#include "semaphore_interface.h"

struct semaphore_t {
	osSemaphoreId_t handle;
};

semaphore_t* semaphore_create(void) {
	semaphore_t* sem = (semaphore_t*)malloc(sizeof(semaphore_t));
	if (sem == NULL) {
		return NULL;  // Handle memory allocation failure
	}

	osSemaphoreAttr_t semAttr = {
		.name = "MySemaphore", // Optional name, can leave NULL if not needed
	};

	sem->handle = osSemaphoreNew(1, 1, &semAttr); // Binary semaphore, count set to 1
	if (sem->handle == NULL) {
		free(sem); // Free memory if semaphore creation fails
		return NULL;
	}

	return sem;
}

void semaphore_take(semaphore_t* sem) {
	if (sem && sem->handle) {
		osSemaphoreAcquire(sem->handle, osWaitForever); // Block until the semaphore is available
	}
}

void semaphore_release(semaphore_t* sem) {
	if (sem && sem->handle) {
		osSemaphoreRelease(sem->handle); // Release the semaphore
	}
}

void semaphore_destroy(semaphore_t* sem) {
	if (sem) {
		if (sem->handle) {
			osSemaphoreDelete(sem->handle); // Delete the CMSIS-RTOS semaphore
		}
		free(sem); // Free the memory allocated for the abstract semaphore
	}
}
