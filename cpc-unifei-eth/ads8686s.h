#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ADS_CONFIGURATION 0x02
#define ADS_CHANNEL_SEL 0x03
#define ADS_RANGE_A1 0x04
#define ADS_RANGE_A2 0x05
#define ADS_RANGE_B1 0x06
#define ADS_RANGE_B2 0x07
#define ADS_STATUS 0x08
#define ADS_OVER_RANGE_SETTING_A 0x0A
#define ADS_OVER_RANGE_SETTING_B 0x0B
#define ADS_LPF_CONFIG 0x0D
#define ADS_DEVICE_ID 0x10
#define ADS_SEQ_STACK_0 0x20
#define ADS_SEQ_STACK_1 0x21
#define ADS_SEQ_STACK_2 0x22
#define ADS_SEQ_STACK_3 0x23
#define ADS_SEQ_STACK_4 0x24
#define ADS_SEQ_STACK_5 0x25
#define ADS_SEQ_STACK_6 0x26
#define ADS_SEQ_STACK_7 0x27
#define ADS_SEQ_STACK_8 0x28
#define ADS_SEQ_STACK_9 0x29
#define ADS_SEQ_STACK_10 0x2A
#define ADS_SEQ_STACK_11 0x2B
#define ADS_SEQ_STACK_12 0x2C
#define ADS_SEQ_STACK_13 0x2D
#define ADS_SEQ_STACK_14 0x2E
#define ADS_SEQ_STACK_15 0x2F
#define ADS_SEQ_STACK_16 0x30
#define ADS_SEQ_STACK_17 0x31
#define ADS_SEQ_STACK_18 0x32
#define ADS_SEQ_STACK_19 0x33
#define ADS_SEQ_STACK_20 0x34
#define ADS_SEQ_STACK_21 0x35
#define ADS_SEQ_STACK_22 0x36
#define ADS_SEQ_STACK_23 0x37
#define ADS_SEQ_STACK_24 0x38
#define ADS_SEQ_STACK_25 0x39
#define ADS_SEQ_STACK_26 0x3A
#define ADS_SEQ_STACK_27 0x3B
#define ADS_SEQ_STACK_28 0x3C
#define ADS_SEQ_STACK_29 0x3D
#define ADS_SEQ_STACK_30 0x3E
#define ADS_SEQ_STACK_31 0x3F

// Function pointer definitions for IO operations
typedef struct {
	void(*set_cs)(void);
	void(*reset_cs)(void);
	void(*reset)(void);  // Function to RESET the ADC
	void(*set_convst)(void);  // Function to set CONVST high
	void(*reset_convst)(void);  // Function to reset CONVST low
	void(*spi_transmit)(uint16_t data);  // Function to transmit data over SPI
	uint16_t(*spi_receive)(void);  // Function to receive data over SPI
	void(*spi_receive_dma)(void);  // Function to reset CONVST low
	void(*on_conversion_complete)(uint16_t result1, uint16_t result2);  // Callback function for when conversion is complete
} ADS8686S_IO;

// Enumeration for the state machine states
typedef enum {
	ADS_STATE_IDLE,
	ADS_STATE_CONVERTING,
	ADS_STATE_WAIT_FOR_BUSY_LOW,
	ADS_STATE_READING_DATA
} ADS8686S_State;

// Structure to hold the state and IO operations for the ADS8686S
typedef struct {
	ADS8686S_IO io;  // IO operations
	ADS8686S_State state;  // Current state
	bool busy_high_flag;  // Internal flag for BUSY high state
	bool busy_low_flag;  // Internal flag for BUSY low state
} ADS8686S;

// Function prototypes
void ADS8686S_Init(ADS8686S_IO *io);  // Initialize the ADS8686S
void ADS8686S_StartConversion(void);  // Start a conversion
void ADS8686S_OnBusyRising(void);  // Function to be called on BUSY rising edge
void ADS8686S_OnBusyFalling(void);  // Function to be called on BUSY falling edge
void ADS8686S_Reset(void);  // Function to RESET the ADC
void ADS8686S_Process(void);  // Function to process the state machine
void ADS8686S_WriteReg(uint8_t regaddr, uint8_t data);
void ADS8686S_ReadReg(uint8_t regaddr, uint16_t *out_data);