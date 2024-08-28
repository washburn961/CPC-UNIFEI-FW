#pragma once

#define IPV4_ADDR(a, b, c, d) ((ipv4){.byte0 = (a), .byte1 = (b), .byte2 = (c), .byte3 = (d)})

#include <stdint.h>

typedef struct
{
	uint8_t byte0;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
} ipv4;

void udp_server_init(void);
void udp_server_send(ipv4 destination_ip, uint16_t destination_port, void *payload, uint16_t payload_length);