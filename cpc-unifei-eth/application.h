#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"
#include "ads8686s.h"
#include "udp_server.h"
#include "gpio.h"
#include "fourier_transform.h"
#include "ring_buffer.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include "ANSI51.h"
#include "ANSI50.h"
#include "ANSI87B.h"
#include "cpc_complex.h"

uint32_t application_init(void);
void application_adc_timing_flag_set(void);
void application_adc_busy_flag_set(void);
uint32_t application_analog_busy_semaphore_can_release(uint16_t GPIO_Pin);

struct ads8686s_conversion_voltage
{
	float channel_a;
	float channel_b;
};