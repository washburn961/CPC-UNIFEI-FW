//#define GOOSE_MCAST_ADDR "01:0C:CD:01:00:00" // Example multicast MAC address for GOOSE
//#define GOOSE_FRAME_SIZE 256 // Adjust based on your GOOSE frame structure
//#define FRAME_SIZE (sizeof(packet_bytes) + 14) // Total size of Ethernet frame
//
//#include "goose_server.h"
//#include "lwip/pbuf.h"
//#include "lwip/netif.h"
//#include "lwip/err.h"
//#include "lwip.h"
//#include <string.h>
//#include "goose.h"
//
//
//goose_handle* handle;
//uint8_t destination[MAC_ADDRESS_SIZE] = { 0x01, 0x0c, 0xcd, 0x01, 0x00, 0x01 };
//uint8_t app_id[APP_ID_SIZE] = { 0x00, 0x05 };
//const char* gocbRef = "CPC UNIFEI/LLN0$GO$TestDataSet";
//const char* dataset = "CPC UNIFEI/LLN0$TestDataSet";
//const char* go_id = "CPC UNIFEI GOID";
//uint16_t time_allowed_to_live = 0;
//uint64_t t = 0; // Time in the format from screenshot (UNIX epoch in nanoseconds)
//uint32_t st_num = 0;
//uint32_t st_num_net = 0;
//uint32_t sq_num = 0;
//uint32_t sq_num_net = 0;
//uint8_t simulation = 0;
//uint8_t conf_rev = 1;
//uint8_t nds_com = 0;
//uint8_t boolean_false = 0;
//uint8_t is_setup = 0;
//uint32_t start_cycles, end_cycles, elapsed_cycles;
//
//void send_goose_test() {
//	struct pbuf *p;
//	
//	if (!is_setup)
//	{
//		handle = goose_init(gnetif.hwaddr, destination, app_id);
//		
//		ber_set(&(handle->frame->pdu_list.gocbref), (uint8_t*)gocbRef, strlen(gocbRef));
//		ber_set(&(handle->frame->pdu_list.dataset), (uint8_t*)dataset, strlen(dataset));
//		ber_set(&(handle->frame->pdu_list.go_id), (uint8_t*)go_id, strlen(go_id));
//		ber_set(&(handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live, sizeof(time_allowed_to_live));
//		ber_set(&(handle->frame->pdu_list.t), (uint8_t*)&t, sizeof(t));
//		ber_set(&(handle->frame->pdu_list.st_num), (uint8_t*)&st_num, sizeof(st_num));
//		ber_set(&(handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num, sizeof(sq_num));
//		ber_set(&(handle->frame->pdu_list.simulation), (uint8_t*)&simulation, sizeof(simulation));
//		ber_set(&(handle->frame->pdu_list.conf_rev), (uint8_t*)&conf_rev, sizeof(conf_rev));
//		ber_set(&(handle->frame->pdu_list.nds_com), (uint8_t*)&nds_com, sizeof(nds_com));
//		
//		for (int i = 0; i < 4; i++)
//		{
//			goose_all_data_entry_add(handle, 0x83, sizeof(boolean_false), &boolean_false);
//		}
//		
//		goose_encode(handle);
//		
//		is_setup = 1;
//	}
//	else
//	{
//		sq_num++;
//		sq_num_net = goose_htonl(sq_num);
//		ber_set(&(handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num_net, sizeof(sq_num_net));
//		goose_encode(handle);
//	}
//
//	LOCK_TCPIP_CORE();
//
//	// Allocate a pbuf for the Ethernet frame
//	p = pbuf_alloc(PBUF_RAW, handle->length, PBUF_RAM);
//	if (p == NULL) {
//		// Handle allocation failure
//		UNLOCK_TCPIP_CORE();
//		return;
//	}
//
//	// Copy the Ethernet frame into the pbuf
//	memcpy(p->payload, handle->byte_stream, handle->length);
//
//	// Send the pbuf using the linkoutput function
//	if (gnetif.linkoutput(&gnetif, p) != ERR_OK) {
//		// Handle send error
//	}
//
//	// Free the pbuf
//	pbuf_free(p);
//    
//	UNLOCK_TCPIP_CORE();
//}
//
////// Function to send GOOSE ping
////void send_goose_test() {
////	struct pbuf *p;
////
////	LOCK_TCPIP_CORE();
////
////	// Fill the Ethernet frame
////	// Destination MAC address (multicast)
////	uint8_t dest_mac[6] = { 0x01, 0x0C, 0xCD, 0x01, 0x00, 0x00 };
////	memcpy(ethernet_frame, dest_mac, 6);
////
////	memcpy(ethernet_frame + 6, gnetif.hwaddr, 6);
////
////	// Ethertype for GOOSE (0x88B8)
////	ethernet_frame[12] = 0x88;
////	ethernet_frame[13] = 0xB8;
////
////	// Copy the payload
////	memcpy(ethernet_frame + 14, packet_bytes, sizeof(packet_bytes));
////
////	// Allocate a pbuf for the Ethernet frame
////	p = pbuf_alloc(PBUF_RAW, FRAME_SIZE, PBUF_RAM);
////	if (p == NULL) {
////		// Handle allocation failure
////		UNLOCK_TCPIP_CORE();
////		return;
////	}
////
////	// Copy the Ethernet frame into the pbuf
////	memcpy(p->payload, ethernet_frame, FRAME_SIZE);
////
////	// Send the pbuf using the linkoutput function
////	if (gnetif.linkoutput(&gnetif, p) != ERR_OK) {
////		// Handle send error
////	}
////
////	// Free the pbuf
////	pbuf_free(p);
////    
////	UNLOCK_TCPIP_CORE();
////}