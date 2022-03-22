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
 * @brief: Removes a task from the beginning of the linked list
 * @param: list - pointer to the linked list
 * @return: dd_task - pointer to the task removed
 */
dd_task_node_t *pop(dd_task_list_t *list) {
    dd_task_node_t *task = list->head;
    list->head = list->head->next;
    list->size--;
    return task;
}

/*
 * remove_task
 * @brief: Removes a task from the linked list
 * @param: list - pointer to the linked list
 * @param: task - pointer to the task to be removed
 * @return: dd_task - pointer to the task removed
 */
dd_task_node_t *remove_task(dd_task_list_t *list, dd_task_node_t *task) {
    dd_task_node_t *curr = list->head;
    dd_task_node_t *prev = NULL;
    while (curr != NULL) {
        if (curr == task) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            list->size--;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

/*
 * remove_task_by_id
 * @brief: Removes a task from the linked list by task id
 * @param: list - pointer to the linked list
 * @param: task_id - task id of the task to be removed
 * @return: dd_task - pointer to the task removed
 */
dd_task_node_t *remove_task_by_id(dd_task_list_t *list, uint32_t task_id) {
    dd_task_node_t *curr = list->head;
    dd_task_node_t *prev = NULL;
    while (curr != NULL) {
        if (curr->task.task_id == task_id) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            list->size--;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

/*
 * switch_elements
 * @brief: Switches the elements of two linked list nodes
 * @param: list - pointer to the linked list
 * @param: node1 - pointer to the first node
 * @param: node2 - pointer to the second node
 * @return: void
 * @note: This function is used by the sort function
 */
void switch_elements(dd_task_list_t *list, dd_task_node_t *node1, dd_task_node_t *node2) {
    dd_task_node_t *prev1 = NULL;
    dd_task_node_t *prev2 = NULL;
    dd_task_node_t *curr = list->head;
    while (curr != NULL) {
        if (curr == node1) {
            if (prev1 == NULL) {
                list->head = node2;
            } else {
                prev1->next = node2;
            }
        }
        if (curr == node2) {
            if (prev2 == NULL) {
                list->head = node1;
            } else {
                prev2->next = node1;
            }
        }
        prev1 = curr;
        curr = curr->next;
    }
}

/*
 * sort_by_deadline
 * @brief: Sorts the linked list by deadline
 * @param: list - pointer to the linked list
 * @return: void
 */
void sort_by_deadline(dd_task_list_t *list) {
    dd_task_node_t *curr = list->head;
    dd_task_node_t *prev = NULL;
    while (curr != NULL) {
        dd_task_node_t *next = curr->next;
        while (next != NULL) {
            if (curr->task.absolute_deadline > next->task.absolute_deadline) {
                switch_elements(list, curr, next);
            }
            prev = next;
            next = next->next;
        }
        curr = curr->next;
    }
}





