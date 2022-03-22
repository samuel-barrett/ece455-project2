#include <stdlib.h>
#include <stdio.h>
//#include <types.h>

#include "linked_list.h"

void init_task_list(dd_task_list_t *list) {
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


/*
 * print_list
 * @brief: Prints the linked list
 * @param: list - pointer to the linked list
 * @return: void
 */
void print_list(dd_task_list_t *list) {
    dd_task_node_t *curr = list->head;
    printf("Task List:\n");
    printf("-----------------------------------------------------\n");
    while (curr != NULL) {
        printf("ID:%d\t\t", curr->task.task_id);
        printf("Type:%d\t\t", curr->task.type);
        printf("Release:%d\t", curr->task.release_time);
        printf("Deadline:%d\t", curr->task.absolute_deadline);
        printf("Completion Time: %d\n", curr->task.completion_time);
        curr = curr->next;
    }
    printf("-----------------------------------------------------\n");
}


//Test 1: Initialize the linked list
void test1(void) {
    printf("\n\nTest 1: Initialize the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);
    if (list.head == NULL && list.size == 0) {
        printf("Test 1: PASSED\n");
    } else {
        printf("Test 1: FAILED\n");
    }
    print_list(&list);
}

//Test 2: Push a task into the linked list
void test2(void) {
    printf("\n\nTest 2: Push a task into the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);
    dd_task_node_t task;
    task.task.task_id = 1;
    task.task.type = PERIODIC;
    task.task.absolute_deadline = 2;
    task.task.completion_time = 3;
    task.task.release_time = 4;

    //Push the task into the linked list
    push(&list, &task);

    //Check if the task is in the linked list
    if (list.head->task.task_id == 1 && list.size == 1) {
        printf("Test 2: PASSED\n");
    } else {
        printf("Test 2: FAILED\n");
    }
    print_list(&list);
}

//Test 3: Push 10 tasks into the linked list
void test3(void) {
    printf("\n\nTest 3: Push multiple tasks into the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);

    //Create array of tasks
    dd_task_node_t tasks[10];

    //Push 10 tasks into the linked list
    for (int i = 0; i < 10; i++) {
        tasks[i].task.task_id = i;
        tasks[i].task.type = PERIODIC;
        tasks[i].task.absolute_deadline = i;
        tasks[i].task.completion_time = i;
        tasks[i].task.release_time = i;
        push(&list, &tasks[i]);
    }

    //Check if the tasks are in the linked list
    if (list.head->task.task_id == 0 && list.size == 10) {
        printf("Test 3: PASSED\n");
    } else {
        printf("Test 3: FAILED\n");
    }
    print_list(&list);
}


int main(int argc, char *argv[]) {
    test1();
    test2();
    test3();
    return 0;
}