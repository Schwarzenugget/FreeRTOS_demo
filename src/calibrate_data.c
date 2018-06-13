/*
 * calibrate_data.c
 *
 * Created: 30/05/2018 9:56:50
 *  Author: David
 */ 
#include "definitions.h"
#include "calibrate_data.h""

void calibrateValues(RGB* bias){
	RGB color;
	while(1){
		
		xQueueReceive(demo_data, &color, portMAX_DELAY);
		
		if(color.R > bias->R){
			color.R -= bias->R;
			}else{
			color.R = 0;
		}
		
		if(color.G > bias->G){
			color.G -= bias->G;
			}else{
			color.G = 0;
		}
		
		if(color.B > bias->B){
			color.B -= bias->B;
		}else{
			color.B = 0;
		}
		
		xQueueSend(calibrated_color, &color, portMAX_DELAY);
	}
}
