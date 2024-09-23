#include "udp_server.h"
#include "udp.h"
#include <string.h>
#include "lwip/apps/lwiperf.h"
#include "command_system.h"

struct udp_pcb *udp_pcb_server;

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	char *buffer = (char *)malloc(p->len + 1);
//	static uint8_t counter = 0;
	if (p != NULL)
	{
		// Send a response back to the sender (optional)
		strncpy(buffer, p->payload, p->len);
		buffer[p->len] = '\0'; // Ensure null termination
		execute_command(buffer, p->len);
		
//		struct pbuf *reply = pbuf_alloc(PBUF_TRANSPORT, p->len, PBUF_RAM);
//		if (reply != NULL)
//		{
//			memcpy(reply->payload, p->payload, p->len);
//			udp_sendto(pcb, reply, addr, port);
//			if (counter++ == 0)
//			{
//				HAL_GPIO_TogglePin(USER_LED2_GPIO_Port, USER_LED2_Pin);
//			}
//			pbuf_free(reply);
//		}
//
//		// Free the received packet buffer
		pbuf_free(p);
	}
}

void udp_server_init(void)
{
	LOCK_TCPIP_CORE();
	
	lwiperf_start_tcp_server_default(NULL, NULL);

	ip4_addr_t remote_addr;
	IP4_ADDR(&remote_addr, 192, 168, 1, 11);
	lwiperf_start_tcp_client_default(&remote_addr, NULL, NULL);
	
	udp_pcb_server = udp_new();
	if (udp_pcb_server != NULL)
	{
		err_t err = udp_bind(udp_pcb_server, IP_ADDR_ANY, 80); // Bind to port 80
		if (err == ERR_OK)
		{
			udp_recv(udp_pcb_server, udp_receive_callback, NULL); // Set up the receive callback
		}
	}
	
	UNLOCK_TCPIP_CORE();
}

void udp_server_send(ipv4 destination_ip, uint16_t destination_port, void *payload, uint16_t payload_length)
{
	LOCK_TCPIP_CORE();
	
	ip_addr_t dest_ip;
	IP4_ADDR(&dest_ip, destination_ip.byte0, destination_ip.byte1, destination_ip.byte2, destination_ip.byte3);
	
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, payload_length, PBUF_RAM);
	
	if (p != NULL)
	{
		// Copy the message into the pbuf
		memcpy(p->payload, payload, payload_length);

		// Send the pbuf using the UDP server PCB
		udp_sendto(udp_pcb_server, p, &dest_ip, destination_port);

		// Free the pbuf
		pbuf_free(p);
	}
	
	UNLOCK_TCPIP_CORE();
}