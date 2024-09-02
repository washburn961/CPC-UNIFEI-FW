#include "ads8686s_low_level.h"
#include "gpio.h"
#include "spi.h"

#define BUFFER_SIZE 16
static volatile uint16_t bufferA[BUFFER_SIZE];
static volatile uint16_t bufferB[BUFFER_SIZE];
static uint16_t tx_nop[BUFFER_SIZE] = { 0 };
static volatile uint16_t *currentBuffer = bufferA;
static volatile uint16_t *processingBuffer = bufferB;

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
	// This function is called from the BUSY falling edge EXTI ISR
	// Start the DMA transfer to read the conversion result
	if (HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)&tx_nop, (uint8_t *)&currentBuffer, 32) != HAL_OK)
	{
		return 1; // Return 1 if there was an error
	}

	return 0;
}

uint32_t ads8686s_low_level_transfer_done(void)
{
	
	return 0;
}