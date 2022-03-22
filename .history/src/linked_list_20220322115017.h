#ifndef LINKED_LIST_H
#define LINKED_LIST_H


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
dd_task_node_t *pop(dd_task_list_t *list);
dd_task_node_t *remove_task(dd_task_list_t *list, dd_task_node_t *task)

#endif
