/*
 * definitions.h
 *
 * Created: 27/04/2018 12:35:40
 *  Author: David M, David S
 */ 
#include <stdlib.h>
#include <string.h>
#include "power_clocks_lib.h"
#include "flashc.h"

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo file headers. */
#include "partest.h"
#include "ethernet.h"
#include "netif/etharp.h"
#include "flash.h"

/*** */
#include "dip204.h"
#include "intc.h"
#include "gpio.h"
#include "pm.h"
#include "delay.h"
#include "spi.h"
#include "conf_clock.h"


/* Priority definitions for most of the tasks in the demo application. */
#define mainLED_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define mainETH_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )

#define mainLCDTEST_PERIOD          ( ( portTickType ) 100 / portTICK_RATE_MS  )

xQueueHandle demo_data;
xQueueHandle calibrated_color;

typedef struct{
	uint16_t R;
	uint16_t G;
	uint16_t B;
}RGB;



