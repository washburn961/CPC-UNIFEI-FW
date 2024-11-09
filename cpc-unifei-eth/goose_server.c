#include "goose_server.h"
#include "cmsis_os.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t destination[MAC_ADDRESS_SIZE] = { 0x01, 0x0c, 0xcd, 0x01, 0x00, 0x01 };
uint8_t app_id[APP_ID_SIZE] = { 0x00, 0x05 };
const char* gocbRef = "UNIFEIDeviceCPC/LLN0$GO$gcb0";
const char* dataset = "UNIFEIDeviceCPC/LLN0$GOOSE0";
const char* go_id = "0005";
uint16_t time_allowed_to_live = 0;
uint64_t t = 0; // Time in the format from screenshot (UNIX epoch in nanoseconds)
uint32_t st_num = 0;
uint32_t st_num_net = 0;
uint32_t sq_num = 0;
uint32_t sq_num_net = 0;
uint8_t simulation = 0;
uint8_t conf_rev = 1;
uint8_t nds_com = 0;

void link_output(uint8_t* byte_stream, size_t length);
void goose_task(void *argument);
uint8_t pdis1_curr_val = 0x0;
uint8_t pdis2_curr_val = 0x0;
uint8_t pdis3_curr_val = 0x0;
uint8_t pioc_curr_val = 0x0;
uint8_t ptoc_curr_val = 0x0;
uint8_t pdift_curr_val = 0x0;
uint8_t pdifb_curr_val = 0x0;
uint8_t pdis1_prev_val = 0x0;
uint8_t pdis2_prev_val = 0x0;
uint8_t pdis3_prev_val = 0x0;
uint8_t pioc_prev_val = 0x0;
uint8_t ptoc_prev_val = 0x0;
uint8_t pdift_prev_val = 0x0;
uint8_t pdifb_prev_val = 0x0;
uint8_t user_button_prev_val = 0x0;
uint8_t user_button_curr_val = 0x0;

goose_message_params input_mon_goose_params;
goose_handle* input_mon_goose_handle;
osThreadId_t goose_task_handle;
const osThreadAttr_t goose_task_attributes = {
	.name = "goose_task",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void goose_server_init(void)
{
	goose_task_handle = osThreadNew(goose_task, NULL, &goose_task_attributes);
}

void goose_field_update(uint32_t field, uint8_t value)
{
	switch (field)
	{
	case PDIS1:
		pdis1_prev_val = pdis1_curr_val;
		pdis1_curr_val = value;
		break;
		
	case PDIS2:
		pdis2_prev_val = pdis2_curr_val;
		pdis2_curr_val = value;
		break;
		
	case PDIS3:
		pdis3_prev_val = pdis3_curr_val;
		pdis3_curr_val = value;
		break;
		
	case PIOC:
		pioc_prev_val = pioc_curr_val;
		pioc_curr_val = value;
		break;
		
	case PTOC:
		ptoc_prev_val = ptoc_curr_val;
		ptoc_curr_val = value;
		break;
		
	case PDIFT:
		pdift_prev_val = pdift_curr_val;
		pdift_curr_val = value;
		break;
		
	case PDIFB:
		pdifb_prev_val = pdifb_curr_val;
		pdifb_curr_val = value;
		break;
		
	default:

		break;
	}
}

void goose_task(void* argument)
{
	uint8_t default_status = 0x0;
	goose_publisher_init(&link_output);
	input_mon_goose_handle = goose_init(gnetif.hwaddr, destination, app_id);
	
	ber_set(&(input_mon_goose_handle->frame->pdu_list.gocbref), (uint8_t*)gocbRef, strlen(gocbRef));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.dataset), (uint8_t*)dataset, strlen(dataset));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.go_id), (uint8_t*)go_id, strlen(go_id));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live, sizeof(time_allowed_to_live));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.t), (uint8_t*)&t, sizeof(t));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.st_num), (uint8_t*)&st_num, sizeof(st_num));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num, sizeof(sq_num));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.simulation), (uint8_t*)&simulation, sizeof(simulation));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.conf_rev), (uint8_t*)&conf_rev, sizeof(conf_rev));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.nds_com), (uint8_t*)&nds_com, sizeof(nds_com));
		
	for (size_t i = 0; i < 7; i++)
	{
		goose_all_data_entry_add(input_mon_goose_handle, 0x83, sizeof(default_status), &default_status);
	}
	
	input_mon_goose_params.default_time_allowed_to_live = 1000;
	input_mon_goose_params.name = "inputMonitoring";
	input_mon_goose_params.handle = input_mon_goose_handle;
	
	goose_publisher_register(input_mon_goose_params);
	
	while (1)
	{
		if (pdis1_curr_val != pdis1_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PDIS1, 0x83, sizeof(uint8_t), &pdis1_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		if (pdis2_curr_val != pdis2_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PDIS2, 0x83, sizeof(uint8_t), &pdis2_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		if (pdis3_curr_val != pdis3_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PDIS3, 0x83, sizeof(uint8_t), &pdis3_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		if (pioc_curr_val != pioc_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PIOC, 0x83, sizeof(uint8_t), &pioc_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		if (ptoc_curr_val != ptoc_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PTOC, 0x83, sizeof(uint8_t), &ptoc_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		if (pdift_curr_val != pdift_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PDIFT, 0x83, sizeof(uint8_t), &pdift_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		if (pdifb_curr_val != pdifb_prev_val)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, PDIFB, 0x83, sizeof(uint8_t), &pdifb_curr_val);
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		goose_publisher_process();
		osDelay(1);
	}
}

void link_output(uint8_t* byte_stream, size_t length)
{
	struct pbuf *p;

	LOCK_TCPIP_CORE();

	// Allocate a pbuf for the Ethernet frame
	p = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
	if (p == NULL) {
		// Handle allocation failure
		UNLOCK_TCPIP_CORE();
		return;
	}

	// Copy the Ethernet frame into the pbuf
	memcpy(p->payload, byte_stream, length);

	// Send the pbuf using the linkoutput function
	if (gnetif.linkoutput(&gnetif, p) != ERR_OK) {
		// Handle send error
		printf("ERRO");
	}

	// Free the pbuf
	pbuf_free(p);
    
	UNLOCK_TCPIP_CORE();
}