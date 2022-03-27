#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

/**
 * @brief Enumeration to determine if the task is a periodic or aperiodic task
 * 
 * @param PERIODIC The task is periodic
 * @param APERIODIC The task is aperiodic
 */
typedef enum task_type {
    PERIODIC,
    APERIODIC
} task_type_t;


/**
 * @brief Struct to hold info about user EDF scheduler tasks
 * 
 * @param (TaskHandle_t) t_handle The task handle
 * @param (uint32_t) task_id The task id
 * @param (task_type_t) type The task type (PERIODIC or APERIODIC)
 * @param (uint32_t) release_time The task release time in ms
 * @param (uint32_t) absolut_deadline The hard absolute deadline in ms
 * @param (uint32_t) completion_time The time it takes to complete the task in ms
 * @param (uint32_t) execution_time The amount of time that the task executes in ms
 * @param (T)
 */
typedef struct dd_task {
    TaskHandle_t t_handle;
    task_type_t type;
    uint32_t task_id;
    uint32_t release_time;
    uint32_t absolute_deadline;
    uint32_t completion_time;
    uint32_t execution_time;
    TaskFunction_t task_function;
} dd_task_t;

/**
 * @brief A node in the linked list
 * 
 * @param task (dd_task_t) The task to store in the node
 * @param next (dd_task_node_t *) Pointer to the next node in the linked list
 */
typedef struct dd_task_node {
    dd_task_t task;
    struct dd_task_node *next;
} dd_task_node_t;

//Struct to hold pointer to the head of the linked list

/**
 * @brief Struct to hold reference to the head of the linked list
 * 
 * @param head (dd_task_node_t *) Pointer to the head of the linked list
 * @param size (uint32_t) The size of the linked list
 */
typedef struct dd_task_list {
    struct dd_task_node *head;
    int size;
} dd_task_list_t;



void init_task_list(dd_task_list_t *list);
dd_task_node_t *get_head(dd_task_list_t *list);
void push(dd_task_list_t *list, dd_task_t task);
dd_task_node_t *pop(dd_task_list_t *list);
TaskHandle_t remove_task(dd_task_list_t *list, uint32_t task_id);
dd_task_t *get_task(dd_task_list_t *list, uint32_t task_id);
void free_list(dd_task_list_t *list);
void print_list(dd_task_list_t *list, char * list_name);


#endif

