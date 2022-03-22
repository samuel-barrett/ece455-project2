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


struct dd_task_list {
    dd_task task;
    struct dd_task_list *next_task; 
};


struct dd_task_list *head = NULL;

void add_task(dd_task task) {
    struct dd_task_list *new_task = (struct dd_task_list *) malloc(sizeof(struct dd_task_list));
    new_task->task = task;
    new_task->next_task = NULL;
    if (head == NULL) {
        head = new_task;
    } else {
        struct dd_task_list *temp = head;
        while (temp->next_task != NULL) {
            temp = temp->next_task;
        }
        temp->next_task = new_task;
    }
}

void print_task_list() {
    struct dd_task_list *temp = head;
    while (temp != NULL) {
        printf("Task ID: %d\n", temp->task.task_id);
        printf("Task Type: %d\n", temp->task.type);
        printf("Release Time: %d\n", temp->task.release_time);
        printf("Absolute Deadline: %d\n", temp->task.absolute_deadline);
        printf("Completion Time: %d\n", temp->task.completion_time);
        temp = temp->next_task;
    }
}

dd_task get_task(uint32_t task_id) {
    struct dd_task_list *temp = head;
    while (temp != NULL) {
        if (temp->task.task_id == task_id) {
            return temp->task;
        }
        temp = temp->next_task;
    }
    return NULL;
}

dd_task get_next_task() {
    struct dd_task_list *temp = head;
    while (temp != NULL) {
        if (temp->task.release_time <= get_current_time()) {
            return temp->task;
        }
        temp = temp->next_task;
    }
    return NULL;
}

dd_task remove_task(uint32_t task_id) {
    struct dd_task_list *temp = head;
    struct dd_task_list *prev = NULL;
    while (temp != NULL) {
        if (temp->task.task_id == task_id) {
            if (prev == NULL) {
                head = temp->next_task;
            } else {
                prev->next_task = temp->next_task;
            }
            dd_task task = temp->task;
            free(temp);
            return task;
        }
        prev = temp;
        temp = temp->next_task;
    }
    return NULL;
}

void sort() {
    struct dd_task_list *temp = head;
    struct dd_task_list *prev = NULL;
    while (temp != NULL) {
        struct dd_task_list *next = temp->next_task;
        while (next != NULL) {
            if (temp->task.absolute_deadline > next->task.absolute_deadline) {
                dd_task temp_task = temp->task;
                temp->task = next->task;
                next->task = temp_task;
            }
            next = next->next_task;
        }
        temp = temp->next_task;
    }
}




