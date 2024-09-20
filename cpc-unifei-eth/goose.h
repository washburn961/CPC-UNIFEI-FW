#pragma once

#include <stdint.h>

#define APP_ID_SIZE 2
#define LEN_SIZE 2
#define RESERVED_1_SIZE 2
#define RESERVED_2_SIZE 2
#define ETHER_TYPE_SIZE 2
#define MAC_ADDRESS_SIZE 6

typedef struct
{
	uint8_t app_id[APP_ID_SIZE];
	uint8_t len[LEN_SIZE];
	uint8_t reserved_1[RESERVED_1_SIZE];
	uint8_t reserved_2[RESERVED_2_SIZE];
} goose_header;

typedef struct
{
	uint8_t *gocbref;
	uint32_t time_allowed_to_live;
	uint8_t *dataset;
	uint8_t *go_id;
	uint64_t t;
	uint32_t st_num;
	uint32_t sq_num;
	uint8_t simulation;
	uint32_t conf_rev;
	uint8_t nds_com;
	uint32_t num_dataset_entries;
	uint8_t *all_data;
} goose_pdu;

typedef struct
{
	uint8_t source_mac_address[MAC_ADDRESS_SIZE];
	uint8_t destination_mac_address[MAC_ADDRESS_SIZE];
	uint8_t ether_type[ETHER_TYPE_SIZE];
	goose_header header;
	goose_pdu pdu;
} goose_frame;