#pragma once

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* ADS8686S CORE */
#define ADS8686S_REG_PCORE_VERSION      0x400
#define ADS8686S_REG_ID                 0x404
#define ADS8686S_REG_UP_SCRATCH         0x408
#define ADS8686S_REG_UP_IF_TYPE         0x40C
#define ADS8686S_REG_UP_CTRL            0x440
#define ADS8686S_REG_UP_CONV_RATE       0x444
#define ADS8686S_REG_UP_BURST_LENGTH    0x448
#define ADS8686S_REG_UP_READ_DATA       0x44C
#define ADS8686S_REG_UP_WRITE_DATA      0x450

/* ADS8686S_REG_UP_CTRL */
#define ADS8686S_CTRL_RESETN            (1 << 0)
#define ADS8686S_CTRL_CNVST_EN          (1 << 1)

#define ADS8686S_REG_CONFIG             0x02
#define ADS8686S_REG_CHANNEL            0x03
#define ADS8686S_REG_INPUT_RANGE_A1     0x04
#define ADS8686S_REG_INPUT_RANGE_A2     0x05
#define ADS8686S_REG_INPUT_RANGE_B1     0x06
#define ADS8686S_REG_INPUT_RANGE_B2     0x07
#define ADS8686S_REG_SEQUENCER_STACK(x) (0x20 + (x))

/* ADS8686S_REG_CONFIG */
#define ADS8686S_SDEF                   (1 << 7)
#define ADS8686S_BURSTEN(x)             ((x & 1) << 6)
#define ADS8686S_BURSTEN_MASK           (1 << 6)
#define ADS8686S_SEQEN(x)               ((x & 1) << 5)
#define ADS8686S_SEQEN_MASK             (1 << 5)
#define ADS8686S_OS(x)                  (((x) & 0x7) << 2)
#define ADS8686S_STATUSEN               (1 << 1)
#define ADS8686S_STATUSEN_MASK          (1 << 1)
#define ADS8686S_CRCEN                  (1 << 0)
#define ADS8686S_CRCEN_MASK             (1 << 0)

/* ADS8686S_REG_CHANNEL */
#define ADS8686S_CHA_MASK               0xF
#define ADS8686S_CHB_MASK               0xF0
#define ADS8686S_CHB_OFFSET             4
#define ADS8686S_CHANNELS_MASK          0xFF

/* ADS8686S_REG_INPUT_RANGE */
#define ADS8686S_INPUT_RANGE(ch, x)     (((x) & 0x3) << (((ch) & 0x3) * 2))

/* ADS8686S_REG_SEQUENCER_STACK(x) */
#define ADS8686S_ADDR(x)                (((x) & 0x7F) << 9)
#define ADS8686S_SSREN                  (1 << 8)
#define ADS8686S_BSEL(x)                (((x) & 0xF) << 4)
#define ADS8686S_ASEL(x)                (((x) & 0xF) << 0)

/* ADS8686S_REG_STATUS */
#define ADS8686S_STATUS_A(x)            (((x) & 0xF) << 12)
#define ADS8686S_STATUS_B(x)            (((x) & 0xF) << 8)
#define ADS8686S_STATUS_CRC(x)          (((x) & 0xFF) << 0)

/* ADS8686S conversion results */
#define ADS8686S_CHANNEL_A_SELF_TEST_VALUE 0xAAAA
#define ADS8686S_CHANNEL_B_SELF_TEST_VALUE 0x5555

/* ADS8686S_REG_PWM */
#define ADS8686S_TRIGGER_PULSE_WIDTH_NS	        50

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
enum ads8686s_channel {
	ADS8686S_VA0,
	ADS8686S_VA1,
	ADS8686S_VA2,
	ADS8686S_VA3,
	ADS8686S_VA4,
	ADS8686S_VA5,
	ADS8686S_VA6,
	ADS8686S_VA7,
	ADS8686S_VA_VCC,
	ADS8686S_VA_ALDO,
	ADS8686S_VA_RESERVED1,
	ADS8686S_VA_SELF_TEST,
	ADS8686S_VA_RESERVED2,
	ADS8686S_VB0,
	ADS8686S_VB1,
	ADS8686S_VB2,
	ADS8686S_VB3,
	ADS8686S_VB4,
	ADS8686S_VB5,
	ADS8686S_VB6,
	ADS8686S_VB7,
	ADS8686S_VB_VCC,
	ADS8686S_VB_ALDO,
	ADS8686S_VB_RESERVED1,
	ADS8686S_VB_SELF_TEST,
	ADS8686S_VB_RESERVED2,
};

enum ads8686s_range {
	ADS8686S_2V5 = 1,
	ADS8686S_5V  = 2,
	ADS8686S_10V = 3,
};

enum ads8686s_osr {
	ADS8686S_OSR_0,
	ADS8686S_OSR_2,
	ADS8686S_OSR_4,
	ADS8686S_OSR_8,
	ADS8686S_OSR_16,
	ADS8686S_OSR_32,
	ADS8686S_OSR_64,
	ADS8686S_OSR_128,
};

extern struct ads8686s_device dev;

struct ads8686s_device
{
	enum ads8686s_range va[8];
	enum ads8686s_range vb[8];
	enum ads8686s_osr osr;
	uint8_t layers_nb;
	uint8_t burst;
	uint8_t init_ok;
	float lsb;
};

struct ads8686s_init_param {
	enum ads8686s_range		va[8];
	enum ads8686s_range		vb[8];
	enum ads8686s_osr		osr;
};

struct ads8686s_conversion_result {
	uint16_t channel_a;
	uint16_t channel_b;
};

struct ads8686s_sequencer_layer {
	enum ads8686s_channel ch_a;
	enum ads8686s_channel ch_b;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/
/* SPI read from device. */
int32_t ads8686s_read(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t *reg_data);
/* SPI write to device. */
int32_t ads8686s_write(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t reg_data);
/* SPI read from device using a mask. */
int32_t ads8686s_read_mask(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t mask,
	uint16_t *data);
/* SPI write to device using a mask. */
int32_t ads8686s_write_mask(struct ads8686s_device *dev,
	uint8_t reg_addr,
	uint16_t mask,
	uint16_t data);
int32_t ads8686s_reset(struct ads8686s_device *dev);
int32_t ads8686s_set_range(struct ads8686s_device *dev,
	enum ads8686s_channel ch,
	enum ads8686s_range range);
/* Set the oversampling ratio. */
int32_t ads8686s_set_oversampling_ratio(struct ads8686s_device *dev,
	enum ads8686s_osr osr);
/* Read data in serial mode. */
int32_t ads8686s_read_data_serial(struct ads8686s_device *dev,
	struct ads8686s_conversion_result *results,
	uint32_t samples);
/* Initialize the device. */
int32_t ads8686s_init(struct ads8686s_device *device, struct ads8686s_init_param *init_param);
/* Read conversion results. */
int32_t ads8686s_read_channel_source(struct ads8686s_device *dev,
	enum ads8686s_channel *ch_a,
	enum ads8686s_channel *ch_b);
/* Select the input source for a channel. */
int32_t ads8686s_select_channel_source(struct ads8686s_device *dev,
	enum ads8686s_channel ch);

int32_t ads8686s_setup_sequencer(struct ads8686s_device *dev,
	struct ads8686s_sequencer_layer *layers,
	uint32_t layers_nb,
	uint8_t burst);

int32_t ads8686s_self_test(struct ads8686s_device *dev);