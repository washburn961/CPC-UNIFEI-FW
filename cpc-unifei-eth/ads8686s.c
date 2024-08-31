#include "ads8686s.h"
#include "spi.h"
#include "string.h"

struct ads8686s_device dev;

static int32_t ads8686s_toggle_conv(void)
{
//	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_RESET);
	
	    // Set the CONVST pin (GPIOA Pin 3)
	GPIOD->BSRR = GPIO_PIN_12; // This sets the pin high
    
	// Reset the CONVST pin (GPIOA Pin 3)
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16; // This resets the pin to low
	
    // Wait for the BUSY pin to go low using direct register access
	while (GPIOD->IDR & GPIO_PIN_11) {
		// Busy wait, do nothing
	}
	
	return 0;
}

/* SPI read from device. */
int32_t ads8686s_read(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t *reg_data)
{
	uint8_t tx_buf[2] = { 0 };
	uint8_t rx_buf[2] = { 0 };
	
	tx_buf[0] = 0x00 | ((reg_addr & 0x3F) << 1);
	tx_buf[1] = 0x00;
	

	HAL_SPI_Transmit(&hspi1, tx_buf, 2, 100);

	HAL_SPI_Receive(&hspi1, rx_buf, 2, 100);
	
	*reg_data = ((rx_buf[0] & 0x01) << 8) | rx_buf[1];
	
	return 0;
}

/* SPI write to device. */
int32_t ads8686s_write(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t reg_data)
{
	uint8_t tx_buf[2] = { 0 };

	tx_buf[0] = 0x80 | ((reg_addr & 0x3F) << 1) | ((reg_data & 0x100) >> 8);
	tx_buf[1] = (reg_data & 0xFF);
	
	HAL_SPI_Transmit(&hspi1, tx_buf, 2, 100);
	
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
	uint8_t	reg_addr;
	uint8_t	mask;
	uint8_t	data;
	uint8_t i;
	int32_t ret;

	if (ch <= ADS8686S_VA7) {
		dev->va[ch] = range;
		if (ch <= ADS8686S_VA3) {
			reg_addr = ADS8686S_REG_INPUT_RANGE_A1;
			mask = ADS8686S_INPUT_RANGE(ch, ADS8686S_10V);
			data = ADS8686S_INPUT_RANGE(ch, range);
		}
		else {
			reg_addr = ADS8686S_REG_INPUT_RANGE_A2;
			mask = ADS8686S_INPUT_RANGE(ch - ADS8686S_VA4, ADS8686S_10V);
			data = ADS8686S_INPUT_RANGE(ch - ADS8686S_VA4, range);
		}
	}
	else {
		dev->vb[ch - ADS8686S_VB0] = range;
		if (ch <= ADS8686S_VB3) {
			reg_addr = ADS8686S_REG_INPUT_RANGE_B1;
			mask = ADS8686S_INPUT_RANGE(ch - ADS8686S_VB0, ADS8686S_10V);
			data = ADS8686S_INPUT_RANGE(ch - ADS8686S_VB0, range);
		}
		else {
			reg_addr = ADS8686S_REG_INPUT_RANGE_B2;
			mask = ADS8686S_INPUT_RANGE(ch - ADS8686S_VB4, ADS8686S_10V);
			data = ADS8686S_INPUT_RANGE(ch - ADS8686S_VB4, range);
		}
	}
	ads8686s_write_mask(dev, reg_addr, mask, data);
	
	return 0;
}

/* Set the oversampling ratio. */
int32_t ads8686s_set_oversampling_ratio(struct ads8686s_device *dev,
	enum ads8686s_osr osr)
{
	int32_t ret = 0;

	ret = ads8686s_write_mask(dev, ADS8686S_REG_CONFIG, ADS8686S_OS(0x7), ADS8686S_OS(osr));

	if (ret != 0)
		return ret;

	dev->osr = osr;
	return 0;
}

static inline uint16_t get_unaligned_be16(const void *p)
{
	const uint8_t *bytes = (const uint8_t *)p;
	return (uint16_t)bytes[0] << 8 | (uint16_t)bytes[1];
}

static int32_t ads8686s_read_channels(struct ads8686s_device *dev,
	struct ads8686s_conversion_result *res)
{
	int32_t ret;
	uint32_t read_nb = dev->layers_nb * 2;
	uint16_t tmp[2] = { 0 };
	
	for (uint32_t i = 0; i < dev->layers_nb; i++, memset(tmp, 0x0, 2))
	{
		HAL_SPI_Receive(&hspi1, (uint8_t *)&tmp, 4, 100);
		tmp[0] = get_unaligned_be16((void *)&tmp[0]);
		tmp[1] = get_unaligned_be16((void *)&tmp[1]);
		res[i].channel_a = tmp[0];
		res[i].channel_b = tmp[1];
	}

//	for (uint32_t i = 0; i < read_nb; i++, tmp = 0) {
//		ret = HAL_SPI_Receive(&hspi1, (uint8_t *)&tmp, 2, 100);
//
//		tmp = get_unaligned_be16((void *)&tmp);
//
//		if (i % 2 == 0)
//			res[i / 2].channel_a = tmp;
//		else
//			res[i / 2].channel_b = tmp;
//	}

	return 0;
}

/* Read data in serial mode. */
int32_t ads8686s_read_data_serial(struct ads8686s_device *dev,
	struct ads8686s_conversion_result *results,
	uint32_t samples)
{
	int32_t ret;

	memset(results, 0, sizeof(*results) * samples);
	
	for (uint32_t i = 0; i < samples; i++) {
		ret = ads8686s_toggle_conv();
		if (ret != 0)
			return ret;

		uint8_t crc;
		ret = ads8686s_read_channels(dev,
			results + (i * dev->layers_nb));
		if (ret != 0)
			return ret;
	}
	return 0;
}

/* Initialize the device. */
int32_t ads8686s_init(struct ads8686s_device *dev, struct ads8686s_init_param *init_param)
{
	uint8_t i;
	
	ads8686s_reset(dev);
	
	for (i = 0; i <= ADS8686S_VA7; i++) {
		dev->va[i] = ADS8686S_10V;
		dev->vb[i] = ADS8686S_10V;
	}
	
//	for (i = 0; i <= ADS8686S_VA7; i++) {
//		dev->va[i] = init_param->va[i] ? init_param->va[i] : ADS8686S_10V;
//		dev->vb[i] = init_param->vb[i] ? init_param->vb[i] : ADS8686S_10V;
//	}
	
	ads8686s_set_oversampling_ratio(dev, init_param->osr);
	
	dev->layers_nb = 1;
	
	ads8686s_self_test(dev);
	
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
	
	ads8686s_read_data_serial(dev, &result, 1);
	
	ads8686s_select_channel_source(dev, ch_save[0]);
	ads8686s_select_channel_source(dev, ch_save[1]);
	
	return ret;
}