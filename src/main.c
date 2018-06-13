/*
 * demo.c
 *
 * Created: 11/05/2018 12:08:19
 *  Author: practiques
 */ 
/* Environment include files. */

#include "definitions.h"
#include "read_sensor.h"
#include "write_data.h"
#include "calibrate_data.h"
/**** */



int main(){
    static pcl_freq_param_t pcl_freq_param =
    {
	    .cpu_f        = configCPU_CLOCK_HZ,
	    .pba_f        = configPBA_CLOCK_HZ,
	    .osc0_f       = FOSC0,
	    .osc0_startup = OSC0_STARTUP
    };

    // Configure system clock
    if (pcl_configure_clocks(&pcl_freq_param) != PASS)
    return 42;
	
	demo_data = xQueueCreate(8, sizeof(RGB));
	calibrated_color = xQueueCreate(8, sizeof(RGB));
	
	
	/* Create tasks */
	xTaskCreate(readValues, (const signed portCHAR *) "readValues", configMINIMAL_STACK_SIZE*5, NULL, 2, NULL);
	
	RGB bias = {0,0,2};
	xTaskCreate(calibrateValues, (const signed portCHAR *) "calibrateValues", configMINIMAL_STACK_SIZE*5, (void*)&bias, 2, NULL);
	
	xTaskCreate(printValues, (const signed portCHAR *) "printValues", configMINIMAL_STACK_SIZE*5, NULL, 2, NULL);

	
	/* Start FreeRTOS */
	vTaskStartScheduler();
	return 0;
}