#include "ads8686s.h"
#include "spi.h"
#include "string.h"

static int32_t ads8686s_toggle_conv(void)
{
	GPIO_PinState busy_pin_state;
	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_RESET);
	
	do
	{
		busy_pin_state = HAL_GPIO_ReadPin(ADC_BUSY_GPIO_Port, ADC_BUSY_Pin);
	} while (busy_pin_state != GPIO_PIN_RESET);
	
	return 0;
}

/* SPI read from device. */
int32_t ads8686s_read(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t *reg_data)
{
	uint8_t tx_buf[2] = { 0 };
	uint8_t rx_buf[2] = { 0 };
	
	tx_buf[0] = 0x00;
	tx_buf[1] = 0x00 | ((reg_addr & 0x3F) << 1);
//	tx_buf[0] = 0x00 | ((reg_addr & 0x3F) << 1);
//	tx_buf[1] = 0x00;
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 1, 1000);
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
	
//	tx_buf[0] = 0x00;
//	tx_buf[1] = 0x00;
//	rx_buf[0] = 0x00;
//	rx_buf[1] = 0x00;
//	//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
//	HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 1, 1000);
//	//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
	
	tx_buf[0] = 0x00;
	tx_buf[1] = 0x00;
	rx_buf[0] = 0x00;
	rx_buf[1] = 0x00;
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 1, 1000);
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
	
	*reg_data = ((rx_buf[0] & 0x01) << 8) | rx_buf[1];
	
	return 0;
}

/* SPI write to device. */
int32_t ads8686s_write(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t reg_data)
{
	uint8_t tx_buf[2] = { 0 };
	uint8_t rx_buf[2] = { 0 };

	tx_buf[0] = 0x80 | ((reg_addr & 0x3F) << 1) | ((reg_data & 0x100) >> 8);
	tx_buf[1] = (reg_data & 0xFF);
	
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 1, 1000);
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
	
	return 0;
}

/* SPI read from device using a mask. */
int32_t ads8686s_read_mask(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t mask,
	uint16_t *data)
{
	uint16_t reg_data;
	
	ads8686s_read(dev, reg_addr, &reg_data);
	*data = (reg_data & mask);
	
	return 0;
}

/* SPI write to device using a mask. */
int32_t ads8686s_write_mask(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t mask,
	uint16_t data)
{
	uint16_t reg_data;
	
	ads8686s_read(dev, reg_addr, &reg_data);
	
	reg_data &= ~mask;
	reg_data |= data;
	
	ads8686s_write(dev, reg_addr, reg_data);
	
	return 0;
}

/* Perform a full reset of the device. */
int32_t ads8686s_reset(struct ads8686s_device *dev)
{
	HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(150);
	
	return 0;
}

/* Set the analog input range for the selected analog input channel. */
int32_t ads8686s_set_range(struct ads8686s_device *dev,
	enum ads8686s_channel ch,
	enum ads8686s_range range)
{
	/* Function implementation goes here */
	return 0;
}

/* Set the oversampling ratio. */
int32_t ads8686s_set_oversampling_ratio(struct ads8686s_device *dev,
	enum ads8686s_osr osr)
{
	/* Function implementation goes here */
	return 0;
}

/* Read data in serial mode. */
int32_t ads8686s_read_data_serial(struct ads8686s_device *dev,
	struct ads8686s_conversion_result *results,
	uint32_t samples)
{
//	memset(results, 0, sizeof(*results) * samples);
//	
//	for (uint32_t i = 0; i < samples; i++) {
//		ads8686s_toggle_conv();
//
//		uint8_t crc;
//		ret = ad7616_read_channels(dev,
//			results + (i * dev->layers_nb),
//			&crc);
//		if (ret != 0)
//			return ret;
//
//		if (dev->crc) {
//			ret = check_crc(results + (i * dev->layers_nb),
//				dev->layers_nb,
//				crc);
//			if (ret != 0)
//				return ret;
//		}
//	}
//	
	return 0;
}

/* Initialize the device. */
int32_t ads8686s_setup(struct ads8686s_device *device)
{
	/* Function implementation goes here */
	ads8686s_reset(device);
	ads8686s_toggle_conv();
	
	return 0;
}

/* Read conversion results. */
int32_t ads8686s_read_channel_source(struct ads8686s_device *dev,
	enum ads8686s_channel *ch_a,
	enum ads8686s_channel *ch_b)
{
	uint16_t val;
	
	ads8686s_read_mask(dev, ADS8686S_REG_CHANNEL, ADS8686S_CHANNELS_MASK, &val);
	
	*ch_a = val & 0xF;
	*ch_b = (val >> 4) + ADS8686S_VB0;
	
	return 0;
}

/* Select the input source for a channel. */
int32_t ads8686s_select_channel_source(struct ads8686s_device *dev,
	enum ads8686s_channel ch)
{
	uint16_t mask = ch >= ADS8686S_VB0 ?  ADS8686S_CHB_MASK : ADS8686S_CHA_MASK;
	uint16_t data = (ch >= ADS8686S_VB0 ? (ch - ADS8686S_VB0) << ADS8686S_CHB_OFFSET : ch);
	
	ads8686s_write_mask(dev, ADS8686S_REG_CHANNEL, mask, data);
	
	return 0;
}

int32_t ads8686s_self_test(struct ads8686s_device *dev)
{
	int ret = 0;
	struct ads8686s_conversion_result result = { 0 };
	enum ads8686s_channel ch_save[2];
	
	ads8686s_read_channel_source(dev, ch_save, ch_save + 1);
	ads8686s_select_channel_source(dev, ADS8686S_VA_SELF_TEST);
	ads8686s_select_channel_source(dev, ADS8686S_VB_SELF_TEST);
	
	ads8686s_toggle_conv();
	
	return ret;
}