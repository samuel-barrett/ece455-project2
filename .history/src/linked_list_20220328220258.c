/**
 * @file linked_list.c
 * @author JJ Carr Cannings, Samuel Barrett
 * @brief This file provides an implementation of a linked list data structure
 *    for use in the EDF scheduler. It is used to store tasks in a linked list
 *    sorted by deadline. It uses dynamic memory allocation to store the nodes.
 * 
 * @version 0.1
 * @date 2022-03-23
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "linked_list.h"

/**
 * @brief Initialize the linked list
 * 
 * @param list (dd_task_list_t *) [IN] Pointer to the linked list
 */
void init_task_list(dd_task_list_t *list) {
    list->head = NULL;
    list->size = 0;
}

/**
 * @brief Return pointer to the head of the linked list
 * 
 * @param list (dd_task_list_t *) [IN] Pointer to the linked list
 * @return dd_task_node_t * Pointer to the head of the linked list
 */
dd_task_node_t *get_head(dd_task_list_t *list) {
    return list->head;
}

/**
 * @brief get the next node in the linked list
 *
 * @param node (dd_task_node_t *) [IN] Pointer to the node in the linked list
 * @return dd_task_node_t * Pointer to the next node in the linked list
 */
dd_task_node_t *get_next(dd_task_node_t *node) {
    return node->next;
}

/**
 * @brief Push a task to the linked list
 * 
 * @param list (dd_task_list_t *) [IN] The linked list to push the task to
 * @param task (dd_task_t) [IN] Task to be pushed
 * @return void
 */
void push(dd_task_list_t *list, dd_task_t task) {
    dd_task_node_t *new_node = (dd_task_node_t *) malloc(sizeof(dd_task_node_t));
    new_node->task = task;
    new_node->next = NULL;
    dd_task_node_t *curr = list->head;
    dd_task_node_t *prev = NULL;

    while(curr != NULL && curr->task.absolute_deadline <= task.absolute_deadline) {
        prev = curr;
        curr = curr->next;
    }
    if (prev == NULL) { //Insert at the head
        new_node->next = list->head;
        list->head = new_node;
    } else {
        prev->next = new_node;
        new_node->next = curr;
    }
    list->size++;
}

/**
 * @brief Get the task object
 * 
 * @param list (dd_task_list_t *) [IN] The linked list to get the task from
 * @param task_id (uint32_t) [IN] The task id to get
 * @return (dd_task_t*) The task object if found, NULL otherwise
 */
dd_task_t *get_task(dd_task_list_t *list, uint32_t task_id) {
    dd_task_node_t *curr = list->head;
    while(curr != NULL) {
        if (curr->task.task_id == task_id) {
            return &curr->task;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief remove a task from the linked list by task id
 * 
 * @param list (dd_task_list_t *) [IN] The linked list to remove the task from
 * @param task_id (uint32_t) [IN] The task id to remove
 * @return (TaskHandle_t) The task handle of the removed task if found, NULL otherwise
 * @note This function uses free() to free the memory allocated to the node
 */
TaskHandle_t remove_task(dd_task_list_t *list, uint32_t task_id) {
    dd_task_node_t *curr = list->head;
    dd_task_node_t *prev = NULL;
    while (curr != NULL) {
        if (curr->task.task_id == task_id) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            TaskHandle_t t_handle = curr->task.t_handle;
            free(curr);
            list->size--;
            return t_handle;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief Free the memory allocated to the linked list and set the size to 0
 * 
 * @param list (dd_task_list_t *) [IN] The linked list to free memory from
 * @return (void)
 */
void free_list(dd_task_list_t *list) {
    dd_task_node_t *curr = list->head;
    while (curr != NULL) {
        dd_task_node_t *next = curr->next;
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->size = 0;
}

/**
 * @brief print items in the linked list from first to last. Prints
 *       the task id, the task type, the task release time, and the
 *       task deadline
 *
 * @param list (dd_task_list_t *) [IN] The linked list to print
 * @return (void)
 */
void print_list(dd_task_list_t *list, char * list_name) {
    dd_task_node_t *curr = list->head;
    printf("%s task list: (size: %d)\n", list_name, list->size);

    printf("UserTID\tRelease\tDeadline\tCompletion\n");
    fflush(stdout);
    while (curr != NULL) {
        //printf("ID:%d\t", curr->task.task_id);
        //printf("Type: %s\t\t", curr->task.type == PERIODIC ? "PERIODIC" : "APERIODIC");
    	printf("\t%d\t\t%d\t\t%d\t\t\t%d\n", curr->task.user_task_id,curr->task.release_time, curr->task.absolute_deadline, curr->task.completion_time);
        curr = get_next(curr);
        fflush(stdout);
    }
}
