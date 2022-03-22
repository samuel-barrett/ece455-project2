/*
 * This file is used to test unit test the linked list implementation
 * It uses the unit test framework provided by the CUnit library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/CUnit.h>

#include "../../src/linked_list.h"

/*Test suite for the linked list implementation*/

//Test 1: Initialize the linked list
void test_init_list(void) {
    dd_task_list_t list;
    init_task_list(&list);
    CU_ASSERT_EQUAL(list.head, NULL);
    CU_ASSERT_EQUAL(list.size, 0);
}

