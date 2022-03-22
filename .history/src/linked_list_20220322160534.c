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
 * @brief: Inserts a task into the linked list in the correct order
 *  i.e by earlist absolute deadline first
 * @param: list - pointer to the linked list
 * @param: task - pointer to the task to be inserted
 * @return: void
 */
void push(dd_task_list_t *list, dd_task_node_t *task) {
    dd_task_node_t *curr = list->head;
    dd_task_node_t *prev = NULL;
    while (curr != NULL && curr->task.absolute_deadline < task->task.absolute_deadline) {
        prev = curr;
        curr = curr->next;
    }
    if (prev == NULL) {
        list->head = task;
    } else {
        prev->next = task;
    }
    task->next = curr;
    list->size++;
}

/*
 * remove_task_by_id
 * @brief: Removes a task from the linked list by task id
 * @param: list - pointer to the linked list
 * @param: task_id - task id of the task to be removed
 * @return: dd_task - pointer to the task removed
 */
dd_task_node_t * remove_task_by_id(dd_task_list_t *list, uint32_t task_id) {
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
    //assert(list.head->task.task_id == 9, "task id is not 9");
    print_list(&list);
}

//Test 4: Pop a task from the linked list by id
void test4(void) {
    printf("\n\nTest 4: Pop a task from the linked list\n");
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

    //Remove the task with id 5
    dd_task_node_t *task = remove_task_by_id(&list, 5);
    
    //Check if the task is in the linked list
    assert(task != NULL, "Task not in the linked list");
    assert(list.size == 9, "Size is not 9");
    print_list(&list);

    //Iterate through the linked list and check if the task is still there
    dd_task_node_t *curr = list.head;
    while (curr != NULL) {
        assert(curr->task.task_id != 5, "Task id is not 5");
        curr = curr->next;
    }

    //Get the id of the first element in the linked list
    int head_id = list.head->task.task_id;

    //Remove the first element in the linked list
    task = remove_task_by_id(&list, head_id);
    printf("Task id: %d\tType: %s\tRelease: %d\tDeadline: %d\tCompletion Time: %d\n", task->task.task_id, task->task.type == PERIODIC ? "PERIODIC" : "APERIODIC", task->task.release_time, task->task.absolute_deadline, task->task.completion_time);
    assert(task != NULL, "Task not in the linked list");
    assert(list.size == 8, "Size is not 8");
    print_list(&list);

    //Iterate through the linked list and check if the task is still there
    curr = list.head;
    while (curr != NULL) {
        assert(curr->task.task_id != head_id, "Task id is wrong");
        curr = curr->next;
    }

    //Get the id of the last element in the linked list
    int tail_id = -1;
    for (int i = 0; i < list.size; i++) {
        tail_id = list.head->task.task_id;
    }
    assert(tail_id != -1, "Didn't find the tail id");

    //Remove the last element in the linked list
    task = remove_task_by_id(&list, tail_id);
    printf("Task id: %d\tType: %s\tRelease: %d\tDeadline: %d\tCompletion Time: %d\n", task->task.task_id, task->task.type == PERIODIC ? "PERIODIC" : "APERIODIC", task->task.release_time, task->task.absolute_deadline, task->task.completion_time);
    assert(task != NULL, "Task not in the linked list");
    assert(list.size == 7, "Size is not 7");
    print_list(&list);

    //Iterate through the linked list and check if the task is still there
    curr = list.head;
    while (curr != NULL) {
        assert(curr->task.task_id != tail_id, "Task id is not %d", tail_id);
        curr = curr->next;
    }

}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    test1();
    test2();
    test3();
    test4();
    return 0;
}