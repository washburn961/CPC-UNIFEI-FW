#include "ads8686s.h"

static ADS8686S ads_context = { 0 };

// Initialize the ADS8686S context
void ADS8686S_Init(ADS8686S_IO *io) {
	ads_context.io = *io;
	ads_context.state = ADS_STATE_IDLE;
	ads_context.busy_high_flag = false;
	ads_context.busy_low_flag = false;
	
	ADS8686S_Reset();
}

// Function to start a conversion
void ADS8686S_StartConversion(void) {
	if (ads_context.state == ADS_STATE_IDLE) {
		ads_context.io.set_convst();  // Start conversion
		ads_context.state = ADS_STATE_CONVERTING;
	}
}

// Function to be called on BUSY rising edge
void ADS8686S_OnBusyRising(void) {
	if (ads_context.state == ADS_STATE_CONVERTING) {
		ads_context.busy_high_flag = true;  // Set the flag for BUSY high
		ads_context.io.reset_convst();
	}
}

// Function to be called on BUSY falling edge
void ADS8686S_OnBusyFalling(void) {
	if (ads_context.state == ADS_STATE_WAIT_FOR_BUSY_LOW) {
		ads_context.busy_low_flag = true;  // Set the flag for BUSY low
	}
}

void ADS8686S_Reset(void)
{
	ads_context.io.reset();
}

// Periodically called function to process the state machine
void ADS8686S_Process(void) {
	if (ads_context.state == ADS_STATE_CONVERTING && ads_context.busy_high_flag) {
		ads_context.busy_high_flag = false;  // Clear the flag
		ads_context.io.reset_convst();  // Set CONVST low
		ads_context.state = ADS_STATE_WAIT_FOR_BUSY_LOW;
	}

	if (ads_context.state == ADS_STATE_WAIT_FOR_BUSY_LOW && ads_context.busy_low_flag) {
		ads_context.busy_low_flag = false;  // Clear the flag
		ads_context.state = ADS_STATE_READING_DATA;
		
		if (ads_context.io.spi_receive_dma)
		{
			ads_context.io.spi_receive_dma();
		}
		else
		{
			// Perform SPI communication to read the ADC results
			uint16_t result1 = ads_context.io.spi_receive();
			uint16_t result2 = ads_context.io.spi_receive();

			// Notify that the conversion is complete
			if (ads_context.io.on_conversion_complete) {
				ads_context.io.on_conversion_complete(result1, result2);
			}
		}

		// Reset to idle state
		ads_context.state = ADS_STATE_IDLE;
	}
}

void ADS8686S_WriteReg(uint8_t regaddr, uint8_t data)
{
	// Create the write command
	uint16_t command = (1 << 15) | ((regaddr & 0x3F) << 9) | (0 << 8) | data;

	// Transmit the command via SPI
//	ads_context.io.set_cs();
	ads_context.io.spi_transmit(command); //0b1011010011111111
//	ads_context.io.reset_cs();
}
void ADS8686S_ReadReg(uint8_t regaddr, uint16_t *out_data)
{
	// Create the read command
	uint16_t command = (0 << 15) | ((regaddr & 0x3F) << 9) | 0x0;
	command = (command >> 8) | (command << 8);

	// Transmit the command via SPI to request a read
//	ads_context.io.set_cs();
	ads_context.io.spi_transmit(command);
	// The next SPI operation will read the data from the ADC
	*out_data = ads_context.io.spi_receive();
//	ads_context.io.reset_cs();
}