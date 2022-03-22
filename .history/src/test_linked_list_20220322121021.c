/*
 * This file is used to test unit test the linked list implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "linked_list.h"

/*Test suite for the linked list implementation*/

//Test 1: Initialize the linked list
void test_init_list(void) {
    dd_task_list_t list;
    init_task_list(&list);
    if (list.head == NULL && list.size == 0) {
        printf("Test 1: PASSED\n");
    } else {
        printf("Test 1: FAILED\n");
    }
}


int main(int argc, char *argv[]) {
    test_init_list();
    return 0;
}

