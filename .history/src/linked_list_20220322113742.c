#include <stdlib.h>
#include <stdio.h>
#include <types.h>


enum task_type {
    PERIODIC,
    APERIODIC
};

struct dd_task {
    TaskHandle_t t_handle;
    task_type type;
    uint32_t task_id;
    uint32_t release_time;
    uint32_t absolute_deadline;
    uint32_t completion_time;
};


struct dd_task_node {
    struct dd_task task;
    struct dd_task_node *next;
};

//Struct to hold pointer to the head of the linked list
struct dd_task_list {
    struct dd_task_node *head;
    int size;
};

void init_task_list(struct dd_task_list *list) {
    list->head = NULL;
    list->size = 0;
}



