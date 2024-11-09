#pragma once

#include "stdint.h"
#include "goose_publisher.h"

#define PDIS1 0
#define PDIS2 1
#define PDIS3 2
#define PIOC 3
#define PTOC 4
#define PDIFT 5
#define PDIFB 6

void goose_server_init(void);
void goose_field_update(uint32_t field, uint8_t value);