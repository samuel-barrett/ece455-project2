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

#define RED_LIGHT_PIN 		GPIO_Pin_0
#define YELLOW_LIGHT_PIN 	GPIO_Pin_1
#define GREEN_LIGHT_PIN 	GPIO_Pin_2
#define ADC_PIN 			GPIO_Pin_3

#define Data_Pin			GPIO_Pin_6
#define Clock_Pin			GPIO_Pin_7
#define Reset_Pin			GPIO_Pin_8

#define NUM_CAR_SPOTS 19
#define STOP_POSITION 8	//Position after stop line where cars should stop if light turns yellow

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

/*
 * Task declarations.
 */
static void Potentiometer_Read_Task( void *pvParameters );
static void Traffic_Generator_Task( void *pvParameters );
static void Traffic_Light_State_Task( TimerHandle_t );
static void System_Display_Task( void *pvParameters );

/*
 * Global queue handles.
 */
xQueueHandle xQueue_potentiometer = 0;
xQueueHandle xQueue_traffic_light = 0;
xQueueHandle xQueue_car_generation = 0;
xTimerHandle xTimer_traffic_light_state = 0;


/*-----------------------------------------------------------*/
/*-------Function Definitions--------------------------------*/
/*-----------------------------------------------------------*/

int init(void){
	GPIO_InitTypeDef gpio_traffic_light;
	GPIO_InitTypeDef gpio_adc;
	ADC_InitTypeDef adc_itd;
	TimerHandle_t timer_handle;

	//Enable Clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//Configure GPIO traffic light pins
	gpio_traffic_light.GPIO_Mode = GPIO_Mode_OUT;
	gpio_traffic_light.GPIO_OType = GPIO_OType_PP;
	gpio_traffic_light.GPIO_Pin =
		GREEN_LIGHT_PIN | YELLOW_LIGHT_PIN | RED_LIGHT_PIN | Data_Pin | Clock_Pin | Reset_Pin;
	gpio_traffic_light.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpio_traffic_light.GPIO_Speed = GPIO_Speed_100MHz;

	//Configure GPIO ADC pin
	gpio_adc.GPIO_Mode = GPIO_Mode_AN;
	gpio_adc.GPIO_Pin = ADC_PIN;
	gpio_adc.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio_adc.GPIO_Speed = GPIO_Speed_100MHz;

	//Initialize GPIOC for traffic light and ADC pins
	GPIO_Init(GPIOC, &gpio_traffic_light);
	GPIO_Init(GPIOC, &gpio_adc);

	//Initialize traffic light to green
	GPIO_SetBits(GPIOC, GREEN_LIGHT_PIN);

	//Configure ADC
	adc_itd.ADC_ContinuousConvMode = 0;
	adc_itd.ADC_DataAlign = ADC_DataAlign_Right;
	adc_itd.ADC_ExternalTrigConv = DISABLE;
	adc_itd.ADC_Resolution = ADC_Resolution_10b;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	//Initialize ADC
	ADC_Init(ADC1, &adc_itd);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_3Cycles);

	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConv(ADC1);
	
	prvSetupHardware();

	//Provide seed for random number generator
	time_t t;
	srand((unsigned) time(&t));

	return 0;
}


int main(void)
{
	//Initialize stuff
	if(!init()){
		printf("Initialized\n");
	}
	else{
		printf("Initialization failed\n");
		return 1;
	}

	//Create queues
	xQueue_potentiometer = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint16_t));
	xQueue_car_generation = xQueueCreate( 	mainQUEUE_LENGTH, sizeof(uint16_t));

	//Create tasks
	vQueueAddToRegistry(xQueue_potentiometer, "PotentiometerQueue");
	vQueueAddToRegistry(xQueue_car_generation, "CarGenerationQueue");

	//Create tasks
	xTaskCreate(Potentiometer_Read_Task, "Potentiometer_Read", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(Traffic_Generator_Task, "Traffic_Generator", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(System_Display_Task, "System_Display", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	//Create timer
	xTimer_traffic_light_state = xTimerCreate("Traffic Light Timer", 1000, pdTRUE, NULL, Traffic_Light_State_Task);
	xTimerStart(xTimer_traffic_light_state, 0);

	//Start the tasks and timer.
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------
/*
 * traffic_light_is_green
 * Description: Reads pin used to set green light and returns state
 * Output: non zero value if green light on, otherwise 0
 */
bool traffic_light_is_green() {
	return GPIO_ReadInputDataBit(GPIOC, GREEN_LIGHT_PIN);
}

/*-----------------------------------------------------------
/*
 * traffic_light_is_yellow
 * Description: Reads pin used to set yellow light and returns state
 * Output: non zero value if yellow light on, otherwise 0
 */
bool traffic_light_is_yellow() {
	return GPIO_ReadInputDataBit(GPIOC, YELLOW_LIGHT_PIN);
}

/*-----------------------------------------------------------
/*
 * traffic_light_is_red
 * Description: Reads pin used to set red light and returns state
 * Output: non zero value if red light on, otherwise 0
 */
bool traffic_light_is_red() {
	return GPIO_ReadInputDataBit(GPIOC, RED_LIGHT_PIN);
}

/*-----------------------------------------------------------
/*
 * Potentionmeter Read Task
 * Description: Reads potentiometer value and puts it in queue, then sleeps for a half a second, and
 * removes the value from the queue.
 * Input: void *pvParameters - pointer to parameters
 * Output: void
 */
static void Potentiometer_Read_Task( void *pvParameters )
{
	uint16_t adc_val, old_adc_val;
	for(;;) {
		ADC_SoftwareStartConv(ADC1);

		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

		//Divide value by 10 so that it is in the range of 0-100
		adc_val = ADC_GetConversionValue(ADC1) / 10;

		//Pop old value off the queue (value not used here)
		xQueueReceive(xQueue_potentiometer, &old_adc_val, 10);

		//Send new adc value into the queue
		xQueueSend(xQueue_potentiometer,&adc_val,1000);

		//Sleep for half a second so that value not being constantly read
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}


/*-----------------------------------------------------------*/
/*
 * Traffic Generator Task
 * Description: Reads potentiometer value from queue, and then uses that value in addition to a
 * random number to determine whether a car should be generated. The higher the value of the
 * potentiometer, the more likely a car will be generated.
 * Input: void *pvParameters - pointer to parameters
 * Output: void
 */
static void  Traffic_Generator_Task( void *pvParameters )
{
	const int POTENTIOMETER_ERR = 3;
	uint16_t potentiometer_val = 0;
	bool is_car = false;
	int rand_val = 0;
	for(;;) {
		//Read potentiometer value from queue
		xQueuePeek(xQueue_potentiometer, &potentiometer_val, 1000);

		//Rescale potentiometer value to linear scale of range (20-100)
		potentiometer_val *= 0.8;
		potentiometer_val += 20;

		//Generate random number between 0 and 100
		rand_val = (rand()*100.0)/RAND_MAX;
		printf("rand: %d --- adc: %u \n", rand_val, potentiometer_val);

		//If random number is greater than potentiometer value, generate car
		//By sending a 1 to the queue, the car generation task will generate a car
		//If random number is less than potentiometer value, do not generate car
		//By sending a 0 to the queue, the car generation task will not generate a car

		if(rand_val <= potentiometer_val+POTENTIOMETER_ERR) { //Create new car
			//Send car to new queue
			is_car = true;
			xQueueSend(xQueue_car_generation, &is_car, 1000);
		} else{
			//Send new value indicating no car
			is_car = false;
			xQueueSend(xQueue_car_generation, &is_car, 1000);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/*-----------------------------------------------------------*/
/*
 * change_light_colour
 * Description: Changes the colour of the traffic light
 * Input: int colour_pin - pin to turn on to change colour
 * Output: void
 */
void change_light_colour(int colour_pin) {
	if (!colour_pin == GREEN_LIGHT_PIN && 
		!colour_pin == YELLOW_LIGHT_PIN && 
		!colour_pin == RED_LIGHT_PIN) {
		printf("Invalid colour pin\n");
		return;
	}

	//Turn off all lights
	GPIO_ResetBits(GPIOC, GREEN_LIGHT_PIN | YELLOW_LIGHT_PIN | RED_LIGHT_PIN);

	//Turn on selected light
	GPIO_SetBits(GPIOC, colour_pin);
}

/*-----------------------------------------------------------*/
/*
 * change_light_period_ms
 * Description: Changes the period of the traffic light timer
 * Input: int period_ms - period of timer in milliseconds
 * Output: void
 */
void change_light_period_ms(int ms_period) {
	xTimerChangePeriod(xTimer_traffic_light_state, pdMS_TO_TICKS(ms_period), 1000);
	printf("Light period changed to %dms\n", ms_period);
}

/*
 * Traffic Light State Task
 * Description: Timer triggered task that changes the state of the traffic light when called.
 * Modifies timer period based on potentiometer value and new traffic light state.
 * Input: TimerHandle_t xTimer - timer handle
 * Output: void
 */
void Traffic_Light_State_Task( TimerHandle_t xTimer ) {
	uint16_t potentiometer_val = 0;

	//Read potentiometer value from queue
	xQueuePeek(xQueue_potentiometer, &potentiometer_val, 1000);


	if(traffic_light_is_green()) {
		//Change to yellow light
		change_light_colour(YELLOW_LIGHT_PIN);

		//Change period of yellow light
		change_light_period_ms(1500)
		printf("Traffic Light Changed to Yellow\n");

	} else if(traffic_light_is_yellow()) {
		//Set light to red
		change_light_colour(RED_LIGHT_PIN);

		//Set timer period to be negatively proportional to potentiometer value
		//When potentiometer value is 0, timer period is 5 seconds
		//When potentiometer value is 100, timer period is 2.5 second
		change_light_period_ms(5000 - potentiometer_val*25);
		printf("Traffic Light Changed to Red\n");
	} else if (traffic_light_is_red()) {
		//Set light to green
		change_light_colour(GREEN_LIGHT_PIN);

		//Set timer period to be proportional to potentiometer value
		//When potentiometer value is 0, timer period is 2.5 second
		//When potentiometer value is 100, timer period is 5 seconds
		change_light_period_ms(2500 + potentiometer_val*25);
		printf("Traffic Light Changed to Green\n");
	} else {
		printf("Traffic Light Error\n");
	}
}



/*-----------------------------------------------------------*/
/*
 * Update Traffic Display
 * Description: updates the traffic display based on the car array by shifting traffic by one
 * Input: boolean is_car - whether a car should be added to the traffic when shifting
 * Output: void
 */
void update_traffic_display(bool add_car)
{
	//If there is a car in the spot, set the data pin high
	if(add_car){
		GPIO_SetBits(GPIOC, Data_Pin);
	}
	GPIO_SetBits(GPIOC, Clock_Pin);
	GPIO_ResetBits(GPIOC, Data_Pin | Clock_Pin);
}

/*-----------------------------------------------------------*/
/*
 * System Display Task
 * Description: task that updates the system display based on the traffic light state and the car queue.
 * Input: void *pvParameters - pointer to parameters
 * Output: void
 */
static void System_Display_Task( void *pvParameters )
{
	bool is_car = false;
	bool traffic_pos[NUM_CAR_SPOTS] = {};

	while(xQueueReceive(xQueue_car_generation, &is_car, 1000)) {

		// Traffic at stop line, shift when traffic light is green
		if(traffic_light_is_green()){
			for(int i=NUM_CAR_SPOTS-1; i>0; --i){
				traffic_pos[i] = traffic_pos[i-1];
			}
			traffic_pos[0] = false;
		} else{ //If light is yellow or red go here

			// Start with traffic beyond stop line, always shift
			for(int i=NUM_CAR_SPOTS-1; i>STOP_POSITION; --i){
				traffic_pos[i] = traffic_pos[i-1];
			}
			//Car after stop line is no longer there
			traffic_pos[STOP_POSITION] = false;

			// shift only up to stop line
			for(int i=STOP_POSITION-2; i>=0; i--){
				if(!traffic_pos[i+1]){ // can only shift forward if no traffic in front
					traffic_pos[i+1] = traffic_pos[i];
					traffic_pos[i] = false;
				}
			}
		}

		// Add new car value if no traffic jam
		traffic_pos[0] = is_car;

		//Reset all the car spots
		GPIO_ResetBits(GPIOC, Reset_Pin);
		GPIO_SetBits(GPIOC, Reset_Pin);

		for(int i=NUM_CAR_SPOTS-1; i>=0; --i) {
			update_traffic_display(traffic_pos[i]);
		}
	}

	fprintf(stderr, "Error, System Display Task failed to receive car from queue\n");
	fprintf(stderr, "System Display Task exiting\n");
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

