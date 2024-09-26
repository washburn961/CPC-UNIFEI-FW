#pragma once

#include "goose.h"

uint8_t destination[MAC_ADDRESS_SIZE] = { 0x01, 0x0c, 0xcd, 0x01, 0x00, 0x01 };
uint8_t app_id[APP_ID_SIZE] = { 0x00, 0x05 };
const char* gocbRef = "UNIFEIDeviceCPC/LLN0$GO$gcb0";
const char* dataset = "UNIFEIDeviceCPC/LLN0$GOOSE0";
const char* go_id = "CPC_GOOSE0";
uint16_t time_allowed_to_live = 0;
uint64_t t = 0; // Time in the format from screenshot (UNIX epoch in nanoseconds)
uint32_t st_num = 0;
uint32_t st_num_net = 0;
uint32_t sq_num = 0;
uint32_t sq_num_net = 0;
uint8_t simulation = 0;
uint8_t conf_rev = 1;
uint8_t nds_com = 0;