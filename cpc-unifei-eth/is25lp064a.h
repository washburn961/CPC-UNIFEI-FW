#pragma once

#include <stdint.h>

void IS25LP064A_Init(void);
void IS25LP064A_Write(uint32_t address, uint8_t *data, uint32_t length);
void IS25LP064A_Read(uint32_t address, uint8_t *out_data, uint32_t length);
void IS25LP064A_WriteReg(uint8_t instruction, uint8_t data);
void IS25LP064A_ReadReg(uint8_t instruction, uint8_t *out_data);
void IS25LP064A_EraseSector(uint32_t address);