/*
 * write_data.c
 *
 * Created: 25/05/2018 11:37:51
 *  Author: practiques
 */ 
#include "write_data.h"
#include "definitions.h"

void display_init2(U32 fcpu_hz)
{
	static const gpio_map_t DIP204_SPI_GPIO_MAP =
	{
		{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION },  // SPI Clock.
		{DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION},  // MISO.
		{DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION},  // MOSI.
		{DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};

	// add the spi options driver structure for the LCD DIP204
	spi_options_t spiOptions =
	{
		.reg          = DIP204_SPI_NPCS,
		.baudrate     = 1000000,
		.bits         = 8,
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI
	gpio_enable_module(DIP204_SPI_GPIO_MAP,
	sizeof(DIP204_SPI_GPIO_MAP) / sizeof(DIP204_SPI_GPIO_MAP[0]));

	// Initialize as master
	spi_initMaster(DIP204_SPI, &spiOptions);

	// Set selection mode: variable_ps, pcs_decode, delay
	spi_selectionMode(DIP204_SPI, 0, 0, 0);

	// Enable SPI
	spi_enable(DIP204_SPI);

	// setup chip registers
	spi_setupChipReg(DIP204_SPI, &spiOptions, fcpu_hz);

	// initialize LCD
	dip204_init(backlight_PWM, true);

	dip204_hide_cursor();
}



void printValues(){
	
	RGB color;
	display_init2(FOSC0*8);
	while(1){
		
		xQueueReceive(calibrated_color, &color, portMAX_DELAY);
		
		//Print R
		dip204_set_cursor_position(1,1);
		dip204_printf_string("         ");
		dip204_set_cursor_position(1,1);
		dip204_printf_string("R: %03d", color.R);
		
		//Print G
		dip204_set_cursor_position(1,2);
		dip204_printf_string("         ");
		dip204_set_cursor_position(1,2);
		dip204_printf_string("G: %03d", color.G);
		
		//Print B
		dip204_set_cursor_position(1,3);
		dip204_printf_string("         ");
		dip204_set_cursor_position(1,3);
		dip204_printf_string("B: %03d", color.B);
		
	}
}