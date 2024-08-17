#include "ads8686s_interface.h"
#include "ads8686s.h"
#include "spi.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define NUM_CHANNELS 16
#define CHANNELS_PER_CONVERSION 2

void ADS8686S_SetCS(void);
void ADS8686S_ResetCS(void);
void ADS8686S_SetCONVST(void);
void ADS8686S_ResetCONVST(void);
void ADS8686S_ResetDevice(void);
void ADS8686S_SPI_Transmit(uint16_t data);
uint16_t ADS8686S_SPI_Receive(void);
void ADS8686S_SPI_Receive_DMA(void);
void ADS8686S_OnConversionComplete(uint16_t result1, uint16_t result2);

ADS8686S_IO io = {
//	.set_cs = ADS8686S_SetCS,
//	.reset_cs = ADS8686S_ResetCS,
	.set_convst = ADS8686S_SetCONVST,
	.reset_convst = ADS8686S_ResetCONVST,
	.spi_transmit = ADS8686S_SPI_Transmit,
	.spi_receive = ADS8686S_SPI_Receive,
	.on_conversion_complete = ADS8686S_OnConversionComplete,
	.spi_receive_dma = ADS8686S_SPI_Receive_DMA,
	.reset = ADS8686S_ResetDevice
};
volatile uint16_t adcResults[NUM_CHANNELS];  // Buffer to store results for all 16 channels
volatile uint16_t conversionBuffer[CHANNELS_PER_CONVERSION];  // Temporary buffer for 2 channels

osThreadId_t adcTaskHandle;
const osThreadAttr_t adcTask_attributes = {
	.name = "adcTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityRealtime,
};

void ADC_Task(void *argument);

void ADS8686S_Interface_Init(void)
{
	ADS8686S_Init(&io);
	uint16_t rx = 0;
	uint16_t tx = (0 << 15) | ((ADS_RANGE_A1 & 0x3F) << 9) | 0x0;
	HAL_StatusTypeDef ret;

	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_RESET);
	
	// Send read register command
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
	ret = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx, (uint8_t*)&rx, 2, 10000);
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);

	// Don't care
	//HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
	ret = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx, (uint8_t*)&rx, 2, 10000);
	//HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);

	// Receive register result
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
	ret = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx, (uint8_t*)&rx, 2, 10000);
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
	
//	ADS8686S_ReadReg(ADS_CONFIGURATION, &reg);
//	adcTaskHandle = osThreadNew(ADC_Task, NULL, &adcTask_attributes);
}

void ADC_Task(void *argument)
{
	for (;;)
	{
		
	}
}

//void ADS8686S_SetCS(void)
//{
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
//}
//void ADS8686S_ResetCS(void)
//{
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
//}

void ADS8686S_SetCONVST(void)
{
	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_SET);
}

void ADS8686S_ResetCONVST(void)
{
	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_RESET);
}

void ADS8686S_ResetDevice(void)
{
	HAL_GPIO_WritePin(ADC_RESET_OD_GPIO_Port, ADC_RESET_OD_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(ADC_RESET_OD_GPIO_Port, ADC_RESET_OD_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

void ADS8686S_SPI_Transmit(uint16_t data)
{
	// Transmit 2 bytes (16-bit data) over SPI
	if (HAL_SPI_Transmit_IT(&hspi1, (uint8_t*)&data, 2) != HAL_OK)
	{
		Error_Handler();
	}
}

uint16_t ADS8686S_SPI_Receive(void)
{
	uint16_t receivedData = 0;
	// Receive 2 bytes (16-bit data) over SPI
	if (HAL_SPI_Receive_IT(&hspi1, (uint8_t*)&receivedData, 2) != HAL_OK)
	{
		Error_Handler();
	}
	return receivedData;
}

void ADS8686S_SPI_Receive_DMA(void)
{
	HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)conversionBuffer, 2);
}

void ADS8686S_OnConversionComplete(uint16_t result1, uint16_t result2)
{
	
}