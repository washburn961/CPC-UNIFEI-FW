#define GOOSE_MCAST_ADDR "01:0C:CD:01:00:00" // Example multicast MAC address for GOOSE
#define GOOSE_FRAME_SIZE 256 // Adjust based on your GOOSE frame structure
#define FRAME_SIZE (sizeof(packet_bytes) + 14) // Total size of Ethernet frame

#include "goose_server.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include <string.h>
#include "goose_frame.h"
//#include "ethernet_frames.h"

// Statically allocated buffer for the Ethernet frame
//static uint8_t ethernet_frame[FRAME_SIZE];

uint8_t is_setup = 0;

void send_goose_test() {
//	struct pbuf *p;
//	
//	if (!is_setup)
//	{
//		
//		is_setup = 1;
//	}
//
//	LOCK_TCPIP_CORE();
//
//	// Allocate a pbuf for the Ethernet frame
//	p = pbuf_alloc(PBUF_RAW, byte_count, PBUF_RAM);
//	if (p == NULL) {
//		// Handle allocation failure
//		UNLOCK_TCPIP_CORE();
//		return;
//	}
//
//	// Copy the Ethernet frame into the pbuf
//	memcpy(p->payload, goose_bytes, byte_count);
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