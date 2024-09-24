#define GOOSE_MCAST_ADDR "01:0C:CD:01:00:00" // Example multicast MAC address for GOOSE
#define GOOSE_FRAME_SIZE 256 // Adjust based on your GOOSE frame structure
#define FRAME_SIZE (sizeof(packet_bytes) + 14) // Total size of Ethernet frame

#include "goose_server.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include <string.h>
#include "goose.h"
#include "goose_frame.h"
//#include "ethernet_frames.h"

// Statically allocated buffer for the Ethernet frame
//static uint8_t ethernet_frame[FRAME_SIZE];

goose_buffer goose_buf;
uint8_t* goose_bytes;
size_t byte_count;

uint8_t is_setup = 0;

void send_goose_test() {
	struct pbuf *p;
	
	if (!is_setup)
	{
		goose_buf.frame.source_mac_address[0] = gnetif.hwaddr[0];
		goose_buf.frame.source_mac_address[1] = gnetif.hwaddr[1];
		goose_buf.frame.source_mac_address[2] = gnetif.hwaddr[2];
		goose_buf.frame.source_mac_address[3] = gnetif.hwaddr[3];
		goose_buf.frame.source_mac_address[4] = gnetif.hwaddr[4];
		goose_buf.frame.source_mac_address[5] = gnetif.hwaddr[5];
	
		goose_buf.frame.destination_mac_address[0] = 0x01;
		goose_buf.frame.destination_mac_address[1] = 0x0C;
		goose_buf.frame.destination_mac_address[2] = 0xCD;
		goose_buf.frame.destination_mac_address[3] = 0x01;
		goose_buf.frame.destination_mac_address[4] = 0x00;
		goose_buf.frame.destination_mac_address[5] = 0x00;
	
		goose_buf.frame.ether_type[0] = 0x88;
		goose_buf.frame.ether_type[1] = 0xb8;
	
		goose_buf.frame.app_id[0] = 0x00;
		goose_buf.frame.app_id[1] = 0x05;
	
		goose_buf.frame.len[0] = 0x00;
		goose_buf.frame.len[1] = 0xa6;
	
		goose_buf.frame.reserved_1[0] = 0x00;
		goose_buf.frame.reserved_1[1] = 0x00;
	
		goose_buf.frame.reserved_2[0] = 0x00;
		goose_buf.frame.reserved_2[1] = 0x00;
	
		goose_set_field(&goose_buf, GOCBREF_FIELD, (uint8_t*)gocbref, sizeof(gocbref));
		goose_set_field(&goose_buf, TIME_ALLOWED_TO_LIVE_FIELD, (uint8_t*)tatl, sizeof(tatl));
		goose_set_field(&goose_buf, DATASET_FIELD, (uint8_t*)dataset, sizeof(dataset));
		goose_set_field(&goose_buf, GOID_FIELD, (uint8_t*)goid, sizeof(goid));
		goose_set_field(&goose_buf, T_FIELD, (uint8_t*)t, sizeof(t));
		goose_set_field(&goose_buf, STNUM_FIELD, (uint8_t*)stnum, sizeof(stnum));
		goose_set_field(&goose_buf, SQNUM_FIELD, (uint8_t*)sqnum, sizeof(sqnum));
		goose_set_field(&goose_buf, SIMULATION_FIELD, (uint8_t*)simulation, sizeof(simulation));
		goose_set_field(&goose_buf, CONFREV_FIELD, (uint8_t*)confrev, sizeof(confrev));
		goose_set_field(&goose_buf, NDSCON_FIELD, (uint8_t*)ndscon, sizeof(ndscon));
		goose_set_field(&goose_buf, NUMDATASETENTRIES_FIELD, (uint8_t*)numdatasetentries, sizeof(numdatasetentries));
		goose_set_field(&goose_buf, ALLDATA_FIELD, (uint8_t*)alldata, sizeof(alldata));
		goose_set_field(&goose_buf, PDU_FIELD, NULL, 0);
		goose_get_bytes(&goose_buf, &goose_bytes, &byte_count);
		is_setup = 1;
	}

	LOCK_TCPIP_CORE();

	// Allocate a pbuf for the Ethernet frame
	p = pbuf_alloc(PBUF_RAW, byte_count, PBUF_RAM);
	if (p == NULL) {
		// Handle allocation failure
		UNLOCK_TCPIP_CORE();
		return;
	}

	// Copy the Ethernet frame into the pbuf
	memcpy(p->payload, goose_bytes, byte_count);

	// Send the pbuf using the linkoutput function
	if (gnetif.linkoutput(&gnetif, p) != ERR_OK) {
		// Handle send error
	}

	// Free the pbuf
	pbuf_free(p);
    
	UNLOCK_TCPIP_CORE();
}

//// Function to send GOOSE ping
//void send_goose_test() {
//	struct pbuf *p;
//
//	LOCK_TCPIP_CORE();
//
//	// Fill the Ethernet frame
//	// Destination MAC address (multicast)
//	uint8_t dest_mac[6] = { 0x01, 0x0C, 0xCD, 0x01, 0x00, 0x00 };
//	memcpy(ethernet_frame, dest_mac, 6);
//
//	memcpy(ethernet_frame + 6, gnetif.hwaddr, 6);
//
//	// Ethertype for GOOSE (0x88B8)
//	ethernet_frame[12] = 0x88;
//	ethernet_frame[13] = 0xB8;
//
//	// Copy the payload
//	memcpy(ethernet_frame + 14, packet_bytes, sizeof(packet_bytes));
//
//	// Allocate a pbuf for the Ethernet frame
//	p = pbuf_alloc(PBUF_RAW, FRAME_SIZE, PBUF_RAM);
//	if (p == NULL) {
//		// Handle allocation failure
//		UNLOCK_TCPIP_CORE();
//		return;
//	}
//
//	// Copy the Ethernet frame into the pbuf
//	memcpy(p->payload, ethernet_frame, FRAME_SIZE);
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