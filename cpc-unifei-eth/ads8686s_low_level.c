#include "ads8686s_low_level.h"
#include "gpio.h"
#include "spi.h"

uint32_t ads8686s_low_level_conversion_start(void)
{
	// Set the CONVST pin
	GPIOD->BSRR = GPIO_PIN_12;
    
	// Reset the CONVST pin
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	return 0;
}

uint32_t ads8686s_low_level_conversion_done(void)
{
	
	return 0;
}