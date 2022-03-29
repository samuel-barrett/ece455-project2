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

//#define TEST_BENCH_1 1
//#define TEST_BENCH_2 2
#define TEST_BENCH_3 3

#ifdef TEST_BENCH_1
	#define TASK1_PERIOD 500
	#define TASK2_PERIOD 500
	#define TASK3_PERIOD 750
	#define TASK1_EXEC_TIME 95
	#define TASK2_EXEC_TIME 150
	#define TASK3_EXEC_TIME 250
#endif

#ifdef TEST_BENCH_2
	#define TASK1_PERIOD 250
	#define TASK2_PERIOD 500
	#define TASK3_PERIOD 750
	#define TASK1_EXEC_TIME 95
	#define TASK2_EXEC_TIME 150
	#define TASK3_EXEC_TIME 250
#endif

#ifdef TEST_BENCH_3
	#define TASK1_PERIOD 500
	#define TASK2_PERIOD 500
	#define TASK3_PERIOD 500
	#define TASK1_EXEC_TIME 100
	#define TASK2_EXEC_TIME 200
	#define TASK3_EXEC_TIME 200
#endif

#define amber_led	LED3
#define green_led	LED4
#define red_led		LED5

#define DDS_PRIORITY				( tskIDLE_PRIORITY + 3 )

#define USER_ACTIVE_TASK_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define USER_IDLE_TASK_PRIORITY		( tskIDLE_PRIORITY + 0)

#define MONITOR_ACTIVE_PRIORITY		( tskIDLE_PRIORITY + 5 )
#define MONITOR_IDLE_PRIORITY		( tskIDLE_PRIORITY + 0 )


#define pdTICKS_TO_MS( xTicks ) ( ( uint32_t ) ( ( ( uint32_t ) ( xTicks ) * ( uint32_t ) 1000 )  / ( uint32_t ) configTICK_RATE_HZ ) )


/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

/*
 * Function declarations.
 */
void complete_dd_task( uint32_t task_id );
dd_task_list_t get_active_dd_task_list(void);
dd_task_list_t get_overdue_dd_task_list(void);
dd_task_list_t get_completed_dd_task_list(void);
void release_dd_task(enum task_type, uint32_t, uint32_t);

/*
 * Task declarations.
 */
static void DDS_Task( void * pvParameters );
void Task_Generator_Task( TimerHandle_t );
static void Monitor_Task( void *pvParameters );

static void User_Defined_Task1( void *pvParameters );
static void User_Defined_Task2( void *pvParameters );
static void User_Defined_Task3( void *pvParameters );

/*
 * Global handles.
 */

xQueueHandle xQueue_new_dd_task = 0;
xQueueHandle xQueue_completed_dd_task = 0;
xQueueHandle xQueue_request_overdue_task_list = 0;
xQueueHandle xQueue_request_completed_task_list = 0;
xQueueHandle xQueue_request_active_task_list = 0;
xQueueHandle xQueue_overdue_task_list = 0;
xQueueHandle xQueue_completed_task_list = 0;
xQueueHandle xQueue_active_task_list = 0;
xTimerHandle xTimer_task1 = 0;
xTimerHandle xTimer_task2 = 0;
xTimerHandle xTimer_task3 = 0;
xSemaphoreHandle monitor_task_lock = 0;


int main(void){
	
	prvSetupHardware();

	STM_EVAL_LEDInit(amber_led);
	STM_EVAL_LEDInit(red_led);
	STM_EVAL_LEDInit(green_led);

	//Create queues
	xQueue_new_dd_task = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_t));
	xQueue_completed_dd_task = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));
	xQueue_request_overdue_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_list_t));
	xQueue_request_completed_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_list_t));
	xQueue_request_active_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_list_t));
	xQueue_overdue_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_list_t));
	xQueue_completed_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_list_t));
	xQueue_active_task_list = xQueueCreate(mainQUEUE_LENGTH, sizeof(dd_task_list_t));
	vQueueAddToRegistry(xQueue_new_dd_task, "NewDDTaskQueue");
	vQueueAddToRegistry(xQueue_completed_dd_task, "CompletedDDTaskQueue");
	vQueueAddToRegistry(xQueue_overdue_task_list, "OverdueTaskListQueue");
	vQueueAddToRegistry(xQueue_completed_task_list, "CompletedTaskListQueue");
	vQueueAddToRegistry(xQueue_active_task_list, "ActiveTaskListQueue");
	vQueueAddToRegistry(xQueue_request_active_task_list, "RequestActiveTaskListQueue");
	vQueueAddToRegistry(xQueue_request_overdue_task_list, "RequestOverdueTaskListQueue");
	vQueueAddToRegistry(xQueue_request_completed_task_list, "RequestCompletedTaskListQueue");

	xTaskCreate(DDS_Task, "DDS_Task", configMINIMAL_STACK_SIZE, NULL, DDS_PRIORITY, NULL);

	//Create timers
	xTimer_task1 = xTimerCreate("Task 1 Timer", 1, pdTRUE, NULL, Task_Generator_Task);
	xTimer_task2 = xTimerCreate("Task 2 Timer", 1, pdTRUE, NULL, Task_Generator_Task);
	xTimer_task3 = xTimerCreate("Task 3 Timer", 1, pdTRUE, NULL, Task_Generator_Task);
	// Start timers
	xTimerStart(xTimer_task1, 0);
	xTimerStart(xTimer_task2, 0);
	xTimerStart(xTimer_task3, 0);

	monitor_task_lock = xSemaphoreCreateBinary();
	xSemaphoreGive(monitor_task_lock);

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
void update_priorities(dd_task_list_t *active_task_list) {
	if(active_task_list->size == 0) {
		return;
	}
	dd_task_node_t *head = get_head(active_task_list);
	vTaskPrioritySet(head->task.t_handle, USER_ACTIVE_TASK_PRIORITY);
	if(active_task_list->size == 1) {
		return;
	}
	dd_task_node_t *curr = get_next(head);
	do {
		//Set other priorities to idle priority
		vTaskPrioritySet(curr->task.t_handle, USER_IDLE_TASK_PRIORITY);
		curr = get_next(curr);
	} while(curr != NULL);
}


void upgrade_monitor_task_priority(TaskHandle_t monitor_t_handle){
	taskENTER_CRITICAL();
	vTaskPrioritySet(monitor_t_handle, MONITOR_ACTIVE_PRIORITY);
	vTaskResume(monitor_t_handle);
	taskYIELD();
	taskEXIT_CRITICAL();
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
 * @return (static void) Does not return.
 */
static void DDS_Task( void *pvParameters )
{
	dd_task_t new_task;
	uint32_t completed_task_id;
	dd_task_list_t active_task_list;
	init_task_list(&active_task_list);
	dd_task_list_t completed_task_list;
	init_task_list(&completed_task_list);
	dd_task_list_t overdue_task_list;
	init_task_list(&overdue_task_list);
	dd_task_list_t tmp_buffer;
	init_task_list(&tmp_buffer);
	uint32_t task_id_cnt = 0;
	TaskHandle_t monitor_t_handle = NULL;

	for(;;){
		if(xQueueReceive(xQueue_new_dd_task, &new_task, 0)){ //New task received
			// Set unique task ID
			new_task.task_id = task_id_cnt;
			// Add new task to active task list and sort by deadline
			push(&active_task_list, new_task);
			dd_task_t *task_list_task = get_task(&active_task_list, new_task.task_id);
			// Create new task in FreeRTOS
			if(task_list_task->user_task_id == 1) {
				xTaskCreate(User_Defined_Task1, "User_Defined_Task1",
								configMINIMAL_STACK_SIZE, task_list_task, USER_IDLE_TASK_PRIORITY, &(task_list_task->t_handle));
			} else if(task_list_task->user_task_id == 2) {
				xTaskCreate(User_Defined_Task2, "User_Defined_Task2",
								configMINIMAL_STACK_SIZE, task_list_task, USER_IDLE_TASK_PRIORITY , &(task_list_task->t_handle));
			} else if(task_list_task->user_task_id == 3) {
				xTaskCreate(User_Defined_Task3, "User_Defined_Task3",
								configMINIMAL_STACK_SIZE, task_list_task, USER_IDLE_TASK_PRIORITY, &(task_list_task->t_handle));
			}else{
				// Aperiodic task
			}
			// Add release time to dd_task
			task_list_task->release_time = pdMS_TO_TICKS(xTaskGetTickCount());
			// Update task priorities in FreeRTOS to reflect EDF sorting
			update_priorities(&active_task_list);

			task_id_cnt++;
		}
		if(xQueueReceive(xQueue_completed_dd_task, &completed_task_id, 0)){ //Task completed
			dd_task_t *completed_task = get_task(&active_task_list, completed_task_id);
			// Remove task from active task list
			remove_task(&active_task_list, completed_task_id);
			// Add task to completed list
			push(&completed_task_list, *completed_task);
			// Add completion time to dd_task struct
			completed_task->completion_time = pdTICKS_TO_MS(xTaskGetTickCount());
			// Update task priorities in FreeRTOS to reflect EDF sorting
			update_priorities(&active_task_list);

			if(xSemaphoreTake(monitor_task_lock, 0)){
				if(!monitor_t_handle){
					xTaskCreate(Monitor_Task, "Monitor_Task", configMINIMAL_STACK_SIZE, NULL, MONITOR_IDLE_PRIORITY, &monitor_t_handle);
				}
				upgrade_monitor_task_priority(monitor_t_handle);
			}
		}
		//Check if any tasks are overdue
		if(active_task_list.size > 0){
			dd_task_node_t *head = get_head(&active_task_list);
			if(xTaskGetTickCount() > head->task.absolute_deadline){ // Task is overdue
				//Add task to overdue list
				push(&overdue_task_list, head->task);
				//Remove task from active task list
				TaskHandle_t overdue_t_handle = remove_task(&active_task_list, head->task.task_id);
				update_priorities(&active_task_list);
				//Delete task from FreeRTOS
				vTaskDelete(overdue_t_handle);
				STM_EVAL_LEDOff(amber_led);
				STM_EVAL_LEDOff(red_led);
				STM_EVAL_LEDOff(green_led);

				if(xSemaphoreTake(monitor_task_lock, 0)){
					if(!monitor_t_handle){
						xTaskCreate(Monitor_Task, "Monitor_Task", configMINIMAL_STACK_SIZE, NULL, MONITOR_IDLE_PRIORITY, &monitor_t_handle);
					}
					upgrade_monitor_task_priority(monitor_t_handle);
				}
			}
		}
		if(xQueueReceive(xQueue_request_active_task_list, &tmp_buffer, 0)){ //Active task list requested
			xQueueSend(xQueue_active_task_list, &active_task_list, 500);
		}
		if(xQueueReceive(xQueue_request_completed_task_list, &tmp_buffer, 0)){ //Completed task list requested
			xQueueSend(xQueue_completed_task_list, &completed_task_list, 500);
		}
		if(xQueueReceive(xQueue_request_overdue_task_list, &tmp_buffer, 0)){ //Overdue task list requested
			xQueueSend(xQueue_overdue_task_list, &overdue_task_list, 500);
		}
		vTaskDelay(1);
	}
}

/**
 * @brief Send a task in the completed task queue, indicating that it has completed
 *
 * @param task_id (uint32_t) [in] ID of task that has completed.
 */
void complete_dd_task( uint32_t task_id )
{
	xQueueSend(xQueue_completed_dd_task, &task_id, 1000);
}


/**
 * @brief This function sends a message to a queue requesting the Active Task
 * List from the DDS. Once a response is received from the DDS, the function
 * returns the list.
 * 
 * @return (dd_task_list_t *) List of active tasks.
 */
dd_task_list_t get_active_dd_task_list(void)
{
	dd_task_list_t active_task_list;
	init_task_list(&active_task_list);
	xQueueSend(xQueue_request_active_task_list, &active_task_list, 1000);
	xQueueReceive(xQueue_active_task_list, &active_task_list, 1000);
	return active_task_list;
}

/**
 * @brief This function sends a message to a queue requesting the Completed
 * Task List from the DDS. Once a response is received from the DDS, the
 * function returns the list.
 * 
 * @return (dd_task_list_t **) List of completed tasks.
 */
dd_task_list_t get_completed_dd_task_list(void) {
	dd_task_list_t completed_task_list;
	init_task_list(&completed_task_list);
	xQueueSend(xQueue_request_completed_task_list, &completed_task_list, 1000);
	xQueueReceive(xQueue_completed_task_list, &completed_task_list, 1000);
	return completed_task_list;
}

/**
 * @brief Get the overdue dd task list object
 * 
 * @return (dd_task_list_t *) List of overdue tasks.
 */
dd_task_list_t get_overdue_dd_task_list(void) {
	dd_task_list_t overdue_task_list;
	init_task_list(&overdue_task_list);
	xQueueSend(xQueue_request_overdue_task_list, &overdue_task_list, 1000);
	xQueueReceive(xQueue_overdue_task_list, &overdue_task_list, 1000);
	return overdue_task_list;
}

/**
 * @brief This function is responsible for monitoring the different task lists
 * 		  and reporting statistics about the tasks. It is a low priority task,
 * 		  and runs in an infinite loop.
 *
 * @param pvParameters (void *) [in] Unused, should be NULL.
 * @return (static void) Does not return.
 */
static void Monitor_Task(void *pvParameters)
{

	dd_task_list_t active_task_list;
	init_task_list(&active_task_list);
	dd_task_list_t completed_task_list;
	init_task_list(&completed_task_list);
	dd_task_list_t overdue_task_list;
	init_task_list(&overdue_task_list);
	for(;;){
		// Request task information from DDS Task
		active_task_list = get_active_dd_task_list();
		completed_task_list = get_completed_dd_task_list();
		overdue_task_list = get_overdue_dd_task_list();
		taskENTER_CRITICAL();
		// Print task information
		printf("Monitor Task | Current Time: %u\n", (uint16_t)pdTICKS_TO_MS(xTaskGetTickCount()));
		print_list(&active_task_list, "Active");
		print_list(&completed_task_list, "Completed");
		print_list(&overdue_task_list, "Overdue");
		printf("-----------------------------\n");

		xSemaphoreGive(monitor_task_lock);
		// Downgrade priority of monitor task to IDLE
		vTaskPrioritySet(xTaskGetCurrentTaskHandle(), MONITOR_IDLE_PRIORITY);
		vTaskSuspend(NULL);
		taskEXIT_CRITICAL();
	}
}

/**
 * @brief This function is used to release a task. It sends a message to the
 * 		  task manager task to the DDS task via a queue to release the task.
 * 
 * @param t_handle (TaskHandle_t) [in] Handle to the task to be released.
 * @param type (task_type_t) [in] Type of task to be released (PERIODIC or APERIODIC).
 * @param execution_time (uint32_t) [in] Execution time of task to be released.
 * @param absolute_deadline (uint32_t) [in] Absolute deadline of task to be released.
 */
void release_dd_task(
	task_type_t type,
	uint32_t user_task_id,
	uint32_t absolute_deadline
){
	dd_task_t new_task;
	new_task.type = type;
	new_task.completion_time = 0;
	new_task.user_task_id = user_task_id;
	new_task.absolute_deadline = absolute_deadline;
	xQueueSend(xQueue_new_dd_task,&new_task,1000);
}

/**
 * @brief This function is used to create periodic tasks and is triggered by a
 * 		  timer. Three different periodic tasks can trigger this function, and 
 *        the timer handle can be used to determine which task is to be released.
 * 
 * @param xTimer (xTimerHandle) [in] Handle to the timer that created the task.
 * @return (void)
 */
void Task_Generator_Task( TimerHandle_t xTimer )
{
	enum task_type t;

	if(xTimer == xTimer_task1){
		t = PERIODIC;
		release_dd_task(t, 1, xTaskGetTickCount()+pdMS_TO_TICKS(TASK1_PERIOD));
		xTimerChangePeriod(xTimer, pdMS_TO_TICKS(TASK1_PERIOD), 1000);
	} else if(xTimer == xTimer_task2){
		t = PERIODIC;
		release_dd_task(t, 2, xTaskGetTickCount()+pdMS_TO_TICKS(TASK2_PERIOD));
		xTimerChangePeriod(xTimer, pdMS_TO_TICKS(TASK2_PERIOD), 1000);
	} else if(xTimer == xTimer_task3){
		t = PERIODIC;
		release_dd_task(t, 3, xTaskGetTickCount()+pdMS_TO_TICKS(TASK3_PERIOD));
		xTimerChangePeriod(xTimer, pdMS_TO_TICKS(TASK3_PERIOD), 1000);
	} else{
		// Aperiodic Implementation
	}

}


/**
 * @brief Application code for tracking the execution of user defined tasks. Turns
 * 		  on the LED amber LED when the task is executing, and turns it off when it
 * 		  completes.
 *
 * @param (void *) pvParameters [in] Task to be executed. Cast to (dd_task_t *)
 */
static void User_Defined_Task1( void * pvParameters)
{
	uint32_t prev_ticks, curr_ticks;

	//Copy values of pvParameters to local variable
	dd_task_t * task = (dd_task_t *)pvParameters;
	STM_EVAL_LEDOn(amber_led);

	uint32_t ticks = pdMS_TO_TICKS(TASK1_EXEC_TIME);

	while(ticks--){
		prev_ticks = xTaskGetTickCount();
		do {
			curr_ticks = xTaskGetTickCount();
		} while(prev_ticks == curr_ticks);

	}
	STM_EVAL_LEDOff(amber_led);

	complete_dd_task(task->task_id);
	vTaskDelete(xTaskGetCurrentTaskHandle());
}

/**
 * @brief Application code for tracking the execution of user defined tasks. Turns
 * 		  on the LED green LED when the task is executing, and turns it off when it
 * 		  completes.
 *
 * @param (void *) pvParameters [in] Task to be executed. Cast to (dd_task_t *)
 * @return (static void)
 */
static void User_Defined_Task2( void * pvParameters)
{
	uint32_t prev_ticks, curr_ticks;
	dd_task_t * task = (dd_task_t *)pvParameters;
	STM_EVAL_LEDOn(green_led);

	uint32_t ticks = pdMS_TO_TICKS(TASK2_EXEC_TIME);

	while(ticks--){
		prev_ticks = xTaskGetTickCount();
		do {
			curr_ticks = xTaskGetTickCount();
		} while(prev_ticks == curr_ticks);

	}
	STM_EVAL_LEDOff(green_led);

	complete_dd_task(task->task_id);
	vTaskDelete(xTaskGetCurrentTaskHandle());
}

/**
 * @brief Application code for tracking the execution of user defined tasks. Turns
 * 		  on the LED red LED when the task is executing, and turns it off when it
 * 		  completes.
 *
 * @param (void *) pvParameters [in] Task to be executed. Cast to (dd_task_t *)
 * @return (static void)
 */
static void User_Defined_Task3( void * pvParameters)
{
	uint32_t prev_ticks, curr_ticks;
	dd_task_t * task = (dd_task_t *)pvParameters;
	STM_EVAL_LEDOn(red_led);

	uint32_t ticks = pdMS_TO_TICKS(TASK3_EXEC_TIME);

	while(ticks--){
		prev_ticks = xTaskGetTickCount();
		do {
			curr_ticks = xTaskGetTickCount();
		} while(prev_ticks == curr_ticks);

	}
	STM_EVAL_LEDOff(red_led);

	complete_dd_task(task->task_id);
	vTaskDelete(xTaskGetCurrentTaskHandle());
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
