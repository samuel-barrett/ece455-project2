#include <stdlib.h>
#include <stdio.h>
//#include <types.h>
//Include random number generator
#include <time.h>

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
 * sort_by_deadline
 * @brief: Sorts the linked list by deadline using bubble sort
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
                if (prev == NULL) {
                    list->head = next;
                } else {
                    prev->next = next;
                }
                curr->next = next->next;
                next->next = curr;
                prev = next;
                curr = curr->next;
                next = curr->next;
            } else {
                prev = curr;
                curr = curr->next;
                next = curr->next;
            }
        }
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
        printf("Type: %s\t\t", curr->task.type == PERIODIC ? "PERIODIC" : "APERIODIC");
        printf("Release:%d\t", curr->task.release_time);
        printf("Deadline:%d\t", curr->task.absolute_deadline);
        printf("Completion Time: %d\n", curr->task.completion_time);
        curr = curr->next;
    }
    printf("-----------------------------------------------------\n");
}


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
    assert(list.head != NULL, "Task not in the linked list");
    assert(list.head->task.task_id == 1, "Task id is not correct");
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
        //Randomly generate the deadline
        tasks[i].task.absolute_deadline = rand() % 30;
        tasks[i].task.completion_time = i;
        tasks[i].task.release_time = i;
        push(&list, &tasks[i]);
    }

    //Check if the tasks are in the linked list
    assert(list.head != NULL, "list head is NULL");
    assert(list.size == 10, "list size is not 10");
    assert(list.head->task.task_id == 9, "task id is not 9");
    print_list(&list);
}


//Test 4: Push 10 tasks into the linked list and sort by deadline
void test4(void) {
    printf("\n\nTest 3: Push multiple tasks into the linked list\n");
    dd_task_list_t list;
    init_task_list(&list);

    //Create array of tasks
    dd_task_node_t tasks[10];

    //Push 10 tasks into the linked list
    for (int i = 0; i < 10; i++) {
        tasks[i].task.task_id = i;
        tasks[i].task.type = PERIODIC;
        //Randomly generate the deadline
        tasks[i].task.absolute_deadline = rand() % 30;
        tasks[i].task.completion_time = i;
        tasks[i].task.release_time = i;
        push(&list, &tasks[i]);
    }
    print_list(&list);
    //Sort the linked list by deadline
    printf("\n\nSorting the linked list by deadline\n");
    sort_by_deadline(&list);
    printf("\n\nSorted linked list\n");

    //Check if the tasks are in the linked in the correct order
    assert(list.head != NULL, "list head is NULL");
    assert(list.size == 10, "list size is not 10");

    //dd_task_node_t *curr = list.head;
    //for (int i = 0; i < 9; i++) {
        //assert(curr->task.absolute_deadline <= curr->next->task.absolute_deadline, "Deadlines are not in order");
        //curr = curr->next;
    //}
    print_list(&list);
}



int main(int argc, char *argv[]) {
    srand(time(NULL));
    test1();
    test2();
    test3();
    test4();
    return 0;
}