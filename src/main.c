/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wwrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
This project simulates a traffic buildup at a traffic light. Here is a description of the project's functionality:

The main() Function:
main() performs initialization and the creates the tasks and software timers described in this section, before
starting the scheduler.




*/

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

/*-----------------------------------------------------------*/
#define mainQUEUE_LENGTH 100
#define TASK1_PERIOD 500
#define TASK2_PERIOD 500
#define TASK3_PERIOD 500
#define TASK1_EXEC_TIME 50
#define TASK2_EXEC_TIME 50
#define TASK3_EXEC_TIME 50


/*
 * Function declarations.
 */
void complete_dd_task( uint32_t task_id );
**dd_task_list get_completed_dd_task_list(void);
**dd_task_list get_active_dd_task_list(void);
**dd_task_list get_overdue_dd_task_list(void);
void release_dd_task(TaskHandle_t, task_type, uint32_t, uint32_t);

/*
 * Task declarations.
 */
static void DDS_Task( void *pvParameters );
static void User_Defined_Tasks_Task( void *pvParameters );
static void Task_Generator_Task( TimerHandle_t );
static void Monitor_Task( void *pvParameters );

/*
 * Global handles.
 */
xQueueHandle xQueue_new_dd_task = 0;
xQueueHandle xQueue_completed_dd_task = 0;
xQueueHandle xQueue_overdue_task_list = 0;
xQueueHandle xQueue_completed_task_list = 0;
xQueueHandle xQueue_active_task_list = 0;
xTimerHandle xTimer_task1 = 0;
xTimerHandle xTimer_task2 = 0;
xTimerHandle xTimer_task3 = 0;

enum task_type {PERIODIC,APERIODIC}; 
 
struct dd_task { 
	TaskHandle_t t_handle;  
	task_type type; 
	uint32_t task_id;  
	uint32_t release_time;  
	uint32_t absolute_deadline; 
	uint32_t completion_time; 
} 

int main(void){
	
	//Create queues
	xQueue_new_dd_task = xQueueCreate(mainQUEUE_LENGTH, sizeof(*struct dd_task));
	xQueue_completed_dd_task = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint16_t));
	xQueue_overdue_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint16_t));
	xQueue_completed_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint16_t));
	xQueue_active_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint16_t));
	vQueueAddToRegistry(xQueue_new_dd_task, "NewDDTaskQueue");
	vQueueAddToRegistry(xQueue_completed_dd_task, "CompletedDDTaskQueue");
	vQueueAddToRegistry(xQueue_overdue_task_list, "OverdueTaskListQueue");
	vQueueAddToRegistry(xQueue_completed_task_list, "CompletedTaskListQueue");
	vQueueAddToRegistry(xQueue_active_task_list, "ActiveTaskListQueue");
	
	//Create timers
	xTimer_task1 = xTimerCreate("Task 1 Timer", pdMS_TO_TICKS(TASK1_PERIOD), pdTRUE, NULL, Task_Generator_Task);
	xTimer_task2 = xTimerCreate("Task 2 Timer", pdMS_TO_TICKS(TASK2_PERIOD), pdTRUE, NULL, Task_Generator_Task);
	xTimer_task3 = xTimerCreate("Task 3 Timer", pdMS_TO_TICKS(TASK3_PERIOD), pdTRUE, NULL, Task_Generator_Task);

	xTaskCreate(DDS_Task, "DDS_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	//xTaskCreate(Task_Generator_Task, "Task_Generator_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(Monitor_Task, "Monitor_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	xTimerStart(xTimer_task1, 0);
	xTimerStart(xTimer_task2, 0);
	xTimerStart(xTimer_task3, 0);
	vTaskStartScheduler();
	
	return 0;
}

static void DDS_Task( void *pvParameters )
{
	struct dd_task *new_task;
	for(;;){
		if(xQueueReceive(xQueue_new_dd_task, &new_task, 50)){
			xTaskCreate(User_Defined_Tasks_Task, "User_Defined_Tasks_Task", configMINIMAL_STACK_SIZE, &new_task, 1, NULL);
		}
	}
}

void complete_dd_task( uint32_t task_id )
{
	xQueueSend(xQueue_completed_dd_task, &task_id, 1000);
}

static void User_Defined_Tasks_Task( dd_task *task )
{
	for(;;){
		
	}
	complete_dd_task(task->task_id);
}

**dd_task_list get_active_dd_task_list(void)
{
	struct dd_task_list **active_task_list;
	xQueueSend(xQueue_active_task_list, &active_task_list, 1000);
	xQueueReceive(xQueue_active_task_list, &active_task_list, 10);
	return active_task_list;
}

**dd_task_list get_completed_dd_task_list(void)
{
	struct dd_task_list **completed_task_list;
	xQueueSend(xQueue_completed_task_list, &completed_task_list, 1000);
	xQueueReceive(xQueue_completed_task_list, &completed_task_list, 10);
	return completed_task_list;
}

**dd_task_list get_overdue_dd_task_list(void)
{
	struct dd_task_list **overdue_task_list;
	xQueueSend(xQueue_overdue_task_list, &overdue_task_list, 1000);
	xQueueReceive(xQueue_overdue_task_list, &overdue_task_list, 10);
	return overdue_task_list;
}

static void Monitor_Task( void *pvParameters )
{
	for(;;){
		
	}
}

void release_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline)
{
	struct dd_task *new_task;
	new_task->t_handle = t_handle;
	new_task->type = type;
	new_task->task_id = task_id;
	new_task->absolute_deadline = absolute_deadline;
	xQueueSend(xQueue_new_dd_task,&new_task,1000);
}

void Task_Generator_Task( TimerHandle_t xTimer )
{
	switch(xTimer)
	{
		case xTimer_task1:
			release_dd_task()
			break;
		case xTimer_task2:
			break;
		case xTimer_task3:
			break;
		default:
			//error
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software 
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
	FreeRTOSConfig.h.

	This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	/* TODO: Setup the clocks, etc. here, if they were not configured before
	main() was called. */
}

