#ifndef OSAL_ERROR_H
#define OSAL_ERROR_H

/**
 * @file osal_error.h
 * @brief OSAL Error Codes for S32K312 Development
 * @details Defines error codes for the Operating System Abstraction Layer (OSAL).
 *          Covers tasks, semaphores, queues, timers, file systems, and more.
 *          Designed for compatibility with BareMetal, FreeRTOS, and AUTOSAR on S32K312.
 */

/* OSAL error base to avoid conflicts with HAL or system errors */
#define OSAL_ERR_BASE                   (0x8000)

/* General OSAL Errors */
#define OSAL_ERR_SUCCESS                (0)                             /* Operation successful */
#define OSAL_ERR_INVALID_PARAM          (OSAL_ERR_BASE + 1)            /* Invalid parameter passed to function */
#define OSAL_ERR_NOT_INITIALIZED        (OSAL_ERR_BASE + 2)            /* Module not initialized */
#define OSAL_ERR_TIMEOUT                (OSAL_ERR_BASE + 3)            /* Operation timed out */
#define OSAL_ERR_RESOURCE_BUSY          (OSAL_ERR_BASE + 4)            /* Resource is currently in use */
#define OSAL_ERR_NOT_SUPPORTED          (OSAL_ERR_BASE + 5)            /* Operation not supported */
#define OSAL_ERR_OUT_OF_MEMORY          (OSAL_ERR_BASE + 6)            /* Memory allocation failed */
#define OSAL_ERR_ALREADY_EXISTS         (OSAL_ERR_BASE + 7)            /* Resource already exists */
#define OSAL_ERR_NOT_FOUND              (OSAL_ERR_BASE + 8)            /* Resource not found */
#define OSAL_ERR_PERMISSION_DENIED      (OSAL_ERR_BASE + 9)            /* Operation not permitted */
#define OSAL_ERR_INTERNAL               (OSAL_ERR_BASE + 10)           /* Internal OSAL error */

/* Task Management Errors */
#define OSAL_ERR_TASK_CREATE_FAILED     (OSAL_ERR_BASE + 20)           /* Failed to create task */
#define OSAL_ERR_TASK_DELETE_FAILED     (OSAL_ERR_BASE + 21)           /* Failed to delete task */
#define OSAL_ERR_TASK_INVALID_ID        (OSAL_ERR_BASE + 22)           /* Invalid task ID */
#define OSAL_ERR_TASK_STACK_OVERFLOW    (OSAL_ERR_BASE + 23)           /* Task stack overflow detected */
#define OSAL_ERR_TASK_PRIORITY_INVALID  (OSAL_ERR_BASE + 24)           /* Invalid task priority */
#define OSAL_ERR_TASK_SUSPEND_FAILED    (OSAL_ERR_BASE + 25)           /* Failed to suspend task */
#define OSAL_ERR_TASK_RESUME_FAILED     (OSAL_ERR_BASE + 26)           /* Failed to resume task */

/* Semaphore Errors */
#define OSAL_ERR_SEM_CREATE_FAILED      (OSAL_ERR_BASE + 30)           /* Failed to create semaphore */
#define OSAL_ERR_SEM_TAKE_FAILED        (OSAL_ERR_BASE + 31)           /* Failed to take semaphore */
#define OSAL_ERR_SEM_GIVE_FAILED        (OSAL_ERR_BASE + 32)           /* Failed to give semaphore */
#define OSAL_ERR_SEM_INVALID_ID         (OSAL_ERR_BASE + 33)           /* Invalid semaphore ID */
#define OSAL_ERR_SEM_COUNT_EXCEEDED     (OSAL_ERR_BASE + 34)           /* Semaphore count exceeded maximum */

/* Queue Errors */
#define OSAL_ERR_QUEUE_CREATE_FAILED    (OSAL_ERR_BASE + 40)           /* Failed to create queue */
#define OSAL_ERR_QUEUE_SEND_FAILED      (OSAL_ERR_BASE + 41)           /* Failed to send to queue */
#define OSAL_ERR_QUEUE_RECEIVE_FAILED   (OSAL_ERR_BASE + 42)           /* Failed to receive from queue */
#define OSAL_ERR_QUEUE_FULL             (OSAL_ERR_BASE + 43)           /* Queue is full */
#define OSAL_ERR_QUEUE_EMPTY            (OSAL_ERR_BASE + 44)           /* Queue is empty */
#define OSAL_ERR_QUEUE_INVALID_ID       (OSAL_ERR_BASE + 45)           /* Invalid queue ID */

/* Timer Errors */
#define OSAL_ERR_TIMER_CREATE_FAILED    (OSAL_ERR_BASE + 50)           /* Failed to create timer */
#define OSAL_ERR_TIMER_START_FAILED     (OSAL_ERR_BASE + 51)           /* Failed to start timer */
#define OSAL_ERR_TIMER_STOP_FAILED      (OSAL_ERR_BASE + 52)           /* Failed to stop timer */
#define OSAL_ERR_TIMER_INVALID_ID       (OSAL_ERR_BASE + 53)           /* Invalid timer ID */
#define OSAL_ERR_TIMER_ALREADY_RUNNING  (OSAL_ERR_BASE + 54)           /* Timer is already running */

/* File System Errors */
#define OSAL_ERR_FS_OPEN_FAILED         (OSAL_ERR_BASE + 60)           /* Failed to open file */
#define OSAL_ERR_FS_READ_FAILED         (OSAL_ERR_BASE + 61)           /* Failed to read from file */
#define OSAL_ERR_FS_WRITE_FAILED        (OSAL_ERR_BASE + 62)           /* Failed to write to file */
#define OSAL_ERR_FS_CLOSE_FAILED        (OSAL_ERR_BASE + 63)           /* Failed to close file */
#define OSAL_ERR_FS_NAME_TOO_LONG       (OSAL_ERR_BASE + 64)           /* File name exceeds maximum length */
#define OSAL_ERR_FS_NO_SPACE            (OSAL_ERR_BASE + 65)           /* No space left on device */
#define OSAL_ERR_FS_INVALID_PATH        (OSAL_ERR_BASE + 66)           /* Invalid file path */
#define OSAL_ERR_FS_NOT_MOUNTED         (OSAL_ERR_BASE + 67)           /* File system not mounted */

/* Interrupt Errors */
#define OSAL_ERR_INT_REGISTER_FAILED    (OSAL_ERR_BASE + 70)           /* Failed to register interrupt handler */
#define OSAL_ERR_INT_UNREGISTER_FAILED  (OSAL_ERR_BASE + 71)           /* Failed to unregister interrupt handler */
#define OSAL_ERR_INT_INVALID_ID         (OSAL_ERR_BASE + 72)           /* Invalid interrupt ID */

/* Mutex Errors */
#define OSAL_ERR_MUTEX_CREATE_FAILED    (OSAL_ERR_BASE + 80)           /* Failed to create mutex */
#define OSAL_ERR_MUTEX_LOCK_FAILED      (OSAL_ERR_BASE + 81)           /* Failed to lock mutex */
#define OSAL_ERR_MUTEX_UNLOCK_FAILED    (OSAL_ERR_BASE + 82)           /* Failed to unlock mutex */
#define OSAL_ERR_MUTEX_INVALID_ID       (OSAL_ERR_BASE + 83)           /* Invalid mutex ID */

/* Event Errors */
#define OSAL_ERR_EVENT_CREATE_FAILED    (OSAL_ERR_BASE + 90)           /* Failed to create event */
#define OSAL_ERR_EVENT_SET_FAILED       (OSAL_ERR_BASE + 91)           /* Failed to set event */
#define OSAL_ERR_EVENT_CLEAR_FAILED     (OSAL_ERR_BASE + 92)           /* Failed to clear event */
#define OSAL_ERR_EVENT_WAIT_FAILED      (OSAL_ERR_BASE + 93)           /* Failed to wait for event */
#define OSAL_ERR_EVENT_INVALID_ID       (OSAL_ERR_BASE + 94)           /* Invalid event ID */

/* Memory Management Errors */
#define OSAL_ERR_MEM_ALLOC_FAILED       (OSAL_ERR_BASE + 100)          /* Memory allocation failed */
#define OSAL_ERR_MEM_FREE_FAILED        (OSAL_ERR_BASE + 101)          /* Memory free failed */
#define OSAL_ERR_MEM_INVALID_PTR        (OSAL_ERR_BASE + 102)          /* Invalid memory pointer */
#define OSAL_ERR_MEM_CORRUPTED          (OSAL_ERR_BASE + 103)          /* Memory corruption detected */

/* Error type */
typedef int32_t osal_err_t;

#endif /* OSAL_ERROR_H */
