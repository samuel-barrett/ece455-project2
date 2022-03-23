/**
 * @file main.c
 * @author JJ Carr Cannings, Samuel Barrett
 * @brief This program is used to create an EDF task scheduler, build on top of the
 * 	  FreeRTOS system, which has real time scheduling capabilities, but does not 
 * 	  have an EDF task scheduler.
 * @version 0.1
 * @date 2022-03-23
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

#include "./linked_list.h"

/*-----------------------------------------------------------*/
#define mainQUEUE_LENGTH 100
#define TASK1_PERIOD 500
#define TASK2_PERIOD 500
#define TASK3_PERIOD 750
#define TASK1_EXEC_TIME 50
#define TASK2_EXEC_TIME 50
#define TASK3_EXEC_TIME 50


/*
 * Function declarations.
 */

void complete_dd_task( uint32_t task_id );
void get_active_dd_task_list(dd_task_list_t * );
void get_overdue_dd_task_list(dd_task_list_t * );
void get_completed_dd_task_list(dd_task_list_t * );
void release_dd_task(TaskHandle_t, enum task_type, uint32_t, uint32_t);

/*
 * Task declarations.
 */

static void DDS_Task( void *pvParameters );
static void User_Defined_Tasks_Task( dd_task_t );
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


int main(void){
	
	//Create queues
	xQueue_new_dd_task = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_t));
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

	xTaskCreate(DDS_Task, "DDS_Task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
	//xTaskCreate(Task_Generator_Task, "Task_Generator_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(Monitor_Task, "Monitor_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	xTimerStart(xTimer_task1, 0);
	xTimerStart(xTimer_task2, 0);
	xTimerStart(xTimer_task3, 0);
	vTaskStartScheduler();
	
	return 0;
}

/**
 * @brief Function changes priority of task with earliest deadline to the 
 * 		highest priority so that it can be executed.
 * 
 * @param active_task_list (dd_task_list_t *) [in] List of active tasks.
 * @return void
 */
void update_priorities(dd_task_list_t *active_task_list)
{
	//TODO: update priorities based on active list order
	uint32_t priority = 1000;
	dd_task_node_t *current_node = get_head(active_task_list);
	while(current_node != NULL){
		vTaskPrioritySet(&(current_node->task.t_handle), priority);
		priority--;
		current_node = current_node->next;
	}
}

/**
 * @brief This task is responsible for creating user defined tasks,
 * 		  and adding them to the active task list. It runs in an infinite
 * 		  loop, and receives values when a new task is created, or when it
 * 		  is completed. 
 * 		  Internally, it keeps track of which tasks are active, completed,
 * 		  and overdue, based on an internal linked list structure.
 * 
 * @param pvParameters (void *) [in] Unused, should be NULL.
 */
static void DDS_Task( void *pvParameters )
{
	dd_task_t new_task;
	uint32_t completed_task_id;
	dd_task_list_t active_task_list;
	dd_task_list_t completed_task_list;
	dd_task_list_t overdue_task_list;
	dd_task_list_t * tmp_buffer;
	uint32_t task_id_cnt = 0;
	for(;;){
		if(xQueueReceive(xQueue_new_dd_task, &new_task, 0)){ //New task received
			new_task.task_id = task_id_cnt;
			// Add new task to active task list and sort by deadline
			push(&active_task_list, new_task);
			dd_task_t *task_list_task = get_task(&active_task_list, new_task.task_id);
			// Create new task in FreeRTOS
			xTaskCreate(User_Defined_Tasks_Task, "User_Defined_Tasks_Task", configMINIMAL_STACK_SIZE, &new_task, 1, &(task_list_task->t_handle));
			update_priorities(&active_task_list);
			// Add release time to dd_task
			task_list_task->release_time = xTaskGetTickCount();
			task_id_cnt++;
		}
		if(xQueueReceive(xQueue_completed_dd_task, &completed_task_id, 0)){ //Task completed
			dd_task_t *completed_task = get_task(&active_task_list, completed_task_id);
			// Remove task from active task list
			remove_task(&active_task_list, completed_task_id);
			// Add task to completed list
			push(&completed_task_list, *completed_task);
			// Delete task from FreeRTOS
			vTaskDelete(&(completed_task->t_handle));
		}
		if(xQueueReceive(xQueue_active_task_list, tmp_buffer, 0)){ //Active task list requested
			xQueueSend(xQueue_active_task_list, &active_task_list, 500);
		}
		if(xQueueReceive(xQueue_completed_task_list, &tmp_buffer, 0)){ //Completed task list requested
			xQueueSend(xQueue_completed_task_list, &completed_task_list, 500);
		}
		if(xQueueReceive(xQueue_overdue_task_list, &tmp_buffer, 0)){ //Overdue task list requested
			xQueueSend(xQueue_overdue_task_list, &overdue_task_list, 500);
		}
		//TODO: Add logic for overdue tasks
	}
}


void complete_dd_task( uint32_t task_id )
{
	xQueueSend(xQueue_completed_dd_task, &task_id, 1000);
}

static void User_Defined_Tasks_Task( dd_task_t task )
{
	for(;;){
		//TODO: Application code & tracking of execution time
	}
	complete_dd_task(task.task_id);
}

void get_active_dd_task_list(dd_task_list_t * active_task_list)
{
	xQueueSend(xQueue_active_task_list, active_task_list, 1000);
	xQueueReceive(xQueue_active_task_list, active_task_list, 1000);
}

void get_completed_dd_task_list(dd_task_list_t * completed_task_list)
{
	xQueueSend(xQueue_completed_task_list, completed_task_list, 1000);
	xQueueReceive(xQueue_completed_task_list, completed_task_list, 1000);
}

void get_overdue_dd_task_list(dd_task_list_t * overdue_task_list)
{
	//Request data
	xQueueSend(xQueue_overdue_task_list, overdue_task_list, 1000);
	//Wait for DDS task to send task list
	xQueueReceive(xQueue_overdue_task_list, overdue_task_list, 1000);
}

static void Monitor_Task( void *pvParameters )
{
	dd_task_list_t active_task_list;
	init_task_list(&active_task_list);
	dd_task_list_t completed_task_list;
	init_task_list(&completed_task_list);
	dd_task_list_t overdue_task_list;
	init_task_list(&overdue_task_list);
	for(;;){
		get_active_dd_task_list(&active_task_list);
		get_completed_dd_task_list(&completed_task_list);
		get_overdue_dd_task_list(&overdue_task_list);

		print_list(&active_task_list);
		print_list(&completed_task_list);
		print_list(&overdue_task_list);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void release_dd_task(TaskHandle_t t_handle, enum task_type type, uint32_t execution_time, uint32_t absolute_deadline)
{
	dd_task_t new_task;
	new_task.t_handle = t_handle;
	new_task.type = type;
	new_task.execution_time = execution_time;
	new_task.absolute_deadline = absolute_deadline;
	xQueueSend(xQueue_new_dd_task,&new_task,1000);
}

void Task_Generator_Task( TimerHandle_t xTimer )
{
	TaskHandle_t xHandle;
	enum task_type t;
	if(xTimer == xTimer_task1){ //TODO: task_id & how to pass in exec. time? (maybe add to dd_task?)
		t = PERIODIC;
		release_dd_task(xHandle, t, TASK1_EXEC_TIME, xTaskGetTickCount()+pdMS_TO_TICKS(TASK1_PERIOD));
	} else if(xTimer == xTimer_task2){
		t = PERIODIC;
		release_dd_task(xHandle, t, TASK2_EXEC_TIME, xTaskGetTickCount()+pdMS_TO_TICKS(TASK2_PERIOD));
	} else if(xTimer == xTimer_task3){
		t = PERIODIC;
		release_dd_task(xHandle, t, TASK3_EXEC_TIME, xTaskGetTickCount()+pdMS_TO_TICKS(TASK3_PERIOD));
	} else {
		t = APERIODIC;
		// Aperiodic?
		//release_dd_task(x_handle, task_type.APERIODIC, , xTaskGetTickCount()+pdMS_TO_TICKS())
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

