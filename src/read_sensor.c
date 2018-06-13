/*
 * read_sensor.c
 *
 * Created: 25/05/2018 11:25:12
 *  Author: practiques
 */ 
#include "read_sensor.h"
#include "definitions.h"

#include "board.h"
#include "compiler.h"
#include "dip204.h"
#include "intc.h"
#include "gpio.h"
#include "pm.h"
#include "delay.h"
#include "spi.h"
#include "conf_clock.h"
#include "twi.h"


#define GESTURE_ADDR 0x39

bool initSensor(void)
{
	uint8_t enable = 0x7; //enable -> write to power on register
	I2C_writeRegister(GESTURE_ADDR, 0x80, &enable);
	
	uint8_t valid = 1;
	I2C_writeRegister(GESTURE_ADDR, 0x93, &valid);
		
	valid = 3;
	I2C_writeRegister(GESTURE_ADDR, 0x8F, &valid); //GAIN
}

void readValues(){
	uint8_t return_value_red, return_value_blue, return_value_green;
	uint16_t red_value, green_value, blue_value;
	uint8_t red_H, red_L, green_H, green_L, blue_H, blue_L;
	
	RGB demo; 
	I2C_Init();
	initSensor();
	while(1){
		
		I2C_readRegister(GESTURE_ADDR, 0x96, &red_L);
		I2C_readRegister(GESTURE_ADDR, 0x97, &red_H);
		red_value = (red_H << 8) | red_L;
		
		I2C_readRegister(GESTURE_ADDR, 0x98, &green_L);
		I2C_readRegister(GESTURE_ADDR, 0x99, &green_H);
		green_value = (green_H << 8) | green_L;
		
		I2C_readRegister(GESTURE_ADDR, 0x9A, &blue_L);
		I2C_readRegister(GESTURE_ADDR, 0x9B, &blue_H);
		blue_value = (blue_H << 8) | blue_L;
		
		demo.R = red_value;
		demo.G = green_value;
		demo.B = blue_value;
		
		xQueueSend(demo_data, &demo, portMAX_DELAY);
		
		vTaskDelay(200);
	}
}