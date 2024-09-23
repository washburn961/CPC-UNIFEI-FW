#pragma once

#include <stdlib.h>
#include <stdint.h>

#define APP_ID_SIZE 2
#define LEN_SIZE 2
#define RESERVED_1_SIZE 2
#define RESERVED_2_SIZE 2
#define ETHER_TYPE_SIZE 2
#define MAC_ADDRESS_SIZE 6

#define GOCBREF_FIELD 0x01
#define TIME_ALLOWED_TO_LIVE_FIELD 0x02
#define DATASET_FIELD 0x03
#define GOID_FIELD 0x04
#define T_FIELD 0x05
#define STNUM_FIELD 0x06
#define SQNUM_FIELD 0x07
#define SIMULATION_FIELD 0x08
#define CONFREV_FIELD 0x09
#define NDSCON_FIELD 0x0A
#define NUMDATASETENTRIES_FIELD 0x0B
#define ALLDATA_FIELD 0x0C
#define PDU_FIELD 0x0D

#define PDU_TAG 0x61
#define GOCBREF_TAG 0x80
#define TIME_ALLOWED_TO_LIVE_TAG 0x81
#define DATASET_TAG 0x82
#define GOID_TAG 0x83
#define T_TAG 0x84
#define STNUM_TAG 0x85
#define SQNUM_TAG 0x86
#define SIMULATION_TAG 0x87
#define CONFREV_TAG 0x88
#define NDSCON_TAG 0x89
#define NUMDATASETENTRIES_TAG 0x8a
#define ALLDATA_TAG 0xab

typedef struct
{
	uint8_t type;
	uint8_t *len;
	uint8_t *value;
	uint32_t len_len;
	uint32_t value_len;
} goose_field;

typedef struct
{
	goose_field gocbref;
	goose_field time_allowed_to_live;
	goose_field dataset;
	goose_field go_id;
	goose_field t;
	goose_field st_num;
	goose_field sq_num;
	goose_field simulation;
	goose_field conf_rev;
	goose_field nds_com;
	goose_field num_dataset_entries;
	goose_field all_data;
} goose_pdu;

typedef struct
{
	goose_field *entries;
} goose_all_data;

typedef struct
{
	uint8_t source_mac_address[MAC_ADDRESS_SIZE];
	uint8_t destination_mac_address[MAC_ADDRESS_SIZE];
	uint8_t ether_type[ETHER_TYPE_SIZE];
	uint8_t app_id[APP_ID_SIZE];
	uint8_t len[LEN_SIZE];
	uint8_t reserved_1[RESERVED_1_SIZE];
	uint8_t reserved_2[RESERVED_2_SIZE];
	goose_field pdu;
} goose_frame;

typedef struct
{
	goose_frame frame;
	uint8_t retransmission_amount;
	uint8_t event_retransmission_period;
} goose_buffer;

uint32_t goose_set_field(goose_buffer* buf, uint8_t field_id, uint8_t* data, uint8_t len);
uint32_t goose_get_bytes(goose_buffer* buf, uint8_t** out_bytes, size_t* out_size);