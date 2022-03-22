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

typedef struct dd_task_list dd_task_list_t;
typedef struct dd_task_node dd_task_node_t;

void init_task_list(dd_task_list_t *list);
    list->head = NULL;
    list->size = 0;
}

/*
 * push
 * @brief: Inserts a task into the linked list
 * @param: list - pointer to the linked list
 * @param: task - pointer to the task to be inserted
 * @return: void
 */
void push(dd_task_list_t *list, dd_task_node_t *task) {
    task->next = list->head;
    list->head = task;
    list->size++;
}

/*
 * pop
 * @brief: Removes a task from the linked list
 * @param: list - pointer to the linked list
 * @return: dd_task - pointer to the task removed
 */
dd_task_node_t *pop(dd_task_list_t *list) {
    dd_task_node_t *task = list->head;
    list->head = list->head->next;
    list->size--;
    return task;
}




