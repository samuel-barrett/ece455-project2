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

    while(curr != NULL && curr->task.absolute_deadline < task.absolute_deadline) {
        prev = curr;
        curr = get_next(curr);
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
        curr = get_next(curr);
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
        curr = get_next(curr);
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
    printf("\n\n%s task list: (size: %d)\n", list_name, list->size);
    printf("-----------------------------------------------------\n");
    while (curr != NULL) {
        printf("ID:%d\t\t", curr->task.task_id);
        printf("Type: %s\t\t", curr->task.type == PERIODIC ? "PERIODIC" : "APERIODIC");
        printf("Release:%d\t", curr->task.release_time);
        printf("Deadline:%d\t", curr->task.absolute_deadline);
        printf("Completion Time: %d\n", curr->task.completion_time);
        curr = get_next(curr);
    }
    printf("-----------------------------------------------------\n");
}

/*
void assert(int condition, char *message) {
    if (!condition) {
        printf("%s\n", message);
        exit(1);
    }
}


//Test 1: Initialize the linked list
void test1(void) {
    printf("\n\nTest 1: Initialize the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);
    assert(list.head == NULL, "Head should be NULL");
    assert(list.size == 0, "Size should be 0");
    print_list(&list);
    free_list(&list);
}

//Test 2: Push a task into the linked list
void test2(void) {
    printf("\n\nTest 2: Push a task into the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);
    dd_task_t task;
    task.task_id = 1;
    task.type = PERIODIC;
    task.absolute_deadline = 2;
    task.completion_time = 3;
    task.release_time = 4;

    //Push the task into the linked list
    push(&list, task);

    //Check if the task is in the linked list
    assert(list.head != NULL, "Task not in the linked list");
    assert(list.head->task.task_id == 1, "Task id is not correct");
    print_list(&list);
    free_list(&list);
}



//Test 3: Push 10 tasks into the linked list
void test3(void) {
    printf("\n\nTest 3: Push multiple tasks into the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);

    //Create array of tasks
    dd_task_t tasks[10];

    //Push 10 tasks into the linked list
    for (int i = 0; i < 10; i++) {
        tasks[i].task_id = i;
        tasks[i].type = PERIODIC;
        //Randomly generate the deadline
        tasks[i].absolute_deadline = rand() % 30;
        tasks[i].completion_time = i;
        tasks[i].release_time = i;
        push(&list, tasks[i]);
    }

    //Check if the tasks are in the linked list
    assert(list.head != NULL, "list head is NULL");
    assert(list.size == 10, "list size is not 10");
    //assert(list.head->task_id == 9, "task id is not 9");
    print_list(&list);
    free_list(&list);
}

//Test 4: Pop a task from the linked list by id
void test4(void) {
    printf("\n\nTest 4: Pop a task from the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);

    //Create array of tasks
    dd_task_t tasks[10];

    //Push 10 tasks into the linked list
    for (int i = 0; i < 10; i++) {
        tasks[i].task_id = i;
        tasks[i].type = PERIODIC;
        //Randomly generate the deadline
        tasks[i].absolute_deadline = rand() % 30;
        tasks[i].completion_time = i;
        tasks[i].release_time = i;
        push(&list, tasks[i]);
    }
    print_list(&list);
    
    //Remove the task with id 5
    printf("Removing task with id 5\n");
    TaskHandle_t t_handle = remove_task(&list, 5);
    printf("List size: %d\n", list.size);
    printf("Task handle: %d\n", t_handle);
    
    //Check if the task is in the linked list
    assert(list.size == 9, "Size is not 9");
    print_list(&list);


    //Iterate through the linked list and check if the task is still there
    dd_task_node_t *curr = list.head;
    while (curr != NULL) {
        assert(curr->task.task_id != 5, "Task id is not 5");
        curr = get_next(curr);
    }

    //Get the id of the first element in the linked list
    int head_id = list.head->task.task_id;

    //Remove the first element in the linked list
    printf("Removing task with id %d\n", head_id);
    t_handle = remove_task(&list, head_id);
    printf("Task handle: %d\n", t_handle);
    assert(list.size == 8, "Size is not 8");
    print_list(&list);

    //Iterate through the linked list and check if the task is still there
    curr = list.head;
    while (curr != NULL) {
        assert(curr->task.task_id != head_id, "Task id is wrong");
        curr = get_next(curr);
    }

    //Get the id of the last element in the linked list
    int tail_id = -1;
    curr = list.head;
    for (int i = 0; i < list.size; i++) {
        tail_id = curr->task.task_id;
        curr = get_next(curr);
    }
    assert(tail_id != -1, "Didn't find the tail id");

    //Remove the last element in the linked list
    printf("Removing tail task %d\n", tail_id);
    t_handle = remove_task(&list, tail_id);
    printf("Task handle: %d\n", t_handle);
    assert(list.size == 7, "Size is not 7");
    print_list(&list);

    //Iterate through the linked list and check if the task is still there
    curr = list.head;
    while (curr != NULL) {
        assert(curr->task.task_id != tail_id, "Task id is wrong");
        curr = get_next(curr);
    }
    free_list(&list);
}

//Test get_task functionaity
void test5(void) {
    printf("\n\nTest 5: Get a task from the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);

    //Create array of tasks
    dd_task_t tasks[10];
    
    //Push 10 tasks into the linked list
    for (int i = 0; i < 10; i++) {
        tasks[i].task_id = i;
        tasks[i].type = PERIODIC;
        //Randomly generate the deadline
        tasks[i].absolute_deadline = rand() % 30;
        tasks[i].completion_time = i;
        tasks[i].release_time = i;
        push(&list, tasks[i]);
    }
    print_list(&list);
    //Get the task with id 5
    printf("Getting task with id 5\n");
    dd_task_t *task = get_task(&list, 5);

    printf("Task id: %d\tTask type: %d\tTask deadline: %d\tTask completion time: %d\tTask release time: %d\n", task->task_id, task->type, task->absolute_deadline, task->completion_time, task->release_time);

    //Check if the task is correct
    assert(task->task_id == 5, "Task id is not 5");
    assert(task->type == PERIODIC, "Task type is not PERIODIC");
    assert(task->completion_time == 5, "Task completion time is not 5");
    assert(task->release_time == 5, "Task release time is not 5");

    free_list(&list);
}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}
*/
