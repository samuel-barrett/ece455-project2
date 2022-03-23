#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
//#include "stm32f4_discovery.h"

/* Kernel includes. */
//#include "stm32f4xx.h"
//#include "../FreeRTOS_Source/include/FreeRTOS.h"
//#include "../FreeRTOS_Source/include/queue.h"
//#include "../FreeRTOS_Source/include/semphr.h"
//#include "../FreeRTOS_Source/include/task.h"
//#include "../FreeRTOS_Source/include/timers.h"

typedef enum task_type {
    PERIODIC,
    APERIODIC
} task_type_t;

/** NOTE **/
// Only uncomment for unit tests
#define TaskHandle_t int

typedef struct dd_task {
    TaskHandle_t t_handle;
    enum task_type type;
    uint32_t task_id;
    uint32_t release_time;
    uint32_t absolute_deadline;
    uint32_t completion_time;
    uint32_t execution_time;
} dd_task_t;


typedef struct dd_task_node {
    struct dd_task task;
    struct dd_task_node *next;
} dd_task_node_t;

//Struct to hold pointer to the head of the linked list
typdef struct dd_task_list {
    struct dd_task_node *head;
    int size;
} dd_task_list_t;



void init_task_list(dd_task_list_t *list);
void push(dd_task_list_t *list, dd_task_t task);
dd_task_node_t *pop(dd_task_list_t *list);
TaskHandle_t remove_task(dd_task_list_t *list, uint32_t task_id);
dd_task_t *get_task(dd_task_list_t *list, uint32_t task_id);
void free_list(dd_task_list_t *list);
void print_list(dd_task_list_t *list);


#endif

