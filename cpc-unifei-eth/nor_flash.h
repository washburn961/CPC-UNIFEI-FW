#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SET_BIT(REG, BIT)    ((REG) |= (BIT))
#define RESET_BIT(REG, BIT)  ((REG) &= ~(BIT))
#define CHECK_BIT(REG, BIT)  ((REG) & (BIT))
#define CHECK_BIT_CLR(REG, BIT)  (!((REG) & (BIT)))

#define PAGE_SIZE 256
#define SECTOR_SIZE 4096
#define BLOCK_SIZE (16 * SECTOR_SIZE)
#define MEMORY_SIZE (SECTOR_SIZE * 2048)

// Status Register Bit Definitions
#define STSREG_WIP        (1 << 0)  // Write In Progress (WIP) bit
#define STSREG_WEL        (1 << 1)  // Write Enable Latch (WEL) bit
#define STSREG_BP0        (1 << 2)  // Block Protection Bit 0 (BP0)
#define STSREG_BP1        (1 << 3)  // Block Protection Bit 1 (BP1)
#define STSREG_BP2        (1 << 4)  // Block Protection Bit 2 (BP2)
#define STSREG_BP3        (1 << 5)  // Block Protection Bit 3 (BP3)
#define STSREG_QE         (1 << 6)  // Quad Enable (QE) bit
#define STSREG_SWRD       (1 << 7)  // Status Register Write Disable (SRWD) bit

// Function Register Bit Definitions
#define FUNCREG_RESERVED      (1 << 0)  // Reserved Bit (Bit 0)
#define FUNCREG_TBS           (1 << 1)  // Top/Bottom Selection (TBS) bit (Bit 1)
#define FUNCREG_PSUS          (1 << 2)  // Program Suspend (PSUS) bit (Bit 2)
#define FUNCREG_ESUS          (1 << 3)  // Erase Suspend (ESUS) bit (Bit 3)
#define FUNCREG_IR_LOCK_0     (1 << 4)  // Information Row Lock 0 (IRL0) bit (Bit 4)
#define FUNCREG_IR_LOCK_1     (1 << 5)  // Information Row Lock 1 (IRL1) bit (Bit 5)
#define FUNCREG_IR_LOCK_2     (1 << 6)  // Information Row Lock 2 (IRL2) bit (Bit 6)
#define FUNCREG_IR_LOCK_3     (1 << 7)  // Information Row Lock 3 (IRL3) bit (Bit 7)

// Read Register Bit Definitions
#define READREG_ODSR2			(1 << 7)  // Output Driver Strength Bit 2 (P7)
#define READREG_ODSR1			(1 << 6)  // Output Driver Strength Bit 1 (P6)
#define READREG_ODSR0			(1 << 5)  // Output Driver Strength Bit 0 (P5)
#define READREG_DUMMY_CYCLES_P4 (1 << 4)  // Dummy Cycles (P4)
#define READREG_DUMMY_CYCLES_P3 (1 << 3)  // Dummy Cycles (P3)
#define READREG_WRAP_ENABLE		(1 << 2)  // Wrap Enable (P2)
#define READREG_BURST_LEN1		(1 << 1)  // Burst Length Bit 1 (P1)
#define READREG_BURST_LEN0		(1 << 0)  // Burst Length Bit 0 (P0)

#define INST_RSTEN				0x66 // Reset Enable
#define INST_RST				0x99 // Reset
#define INST_WREN				0x06 // Write Enable
#define INST_RDSR				0x05 // Read Status Register
#define INST_WRSR				0x01 // Write Status Register
#define INST_QPIEN				0x35 // Enter Quad Peripheral Interface
#define INST_PP					0x02 // Page Program Operation
#define INST_SRP				0xC0 // Set Read Parameters Operation
#define INST_FRD				0x0B // Fast Read Operation
#define INST_SER				0x20 // Sector Erase Operation

typedef enum
{
	SPI  = 0,
	QSPI = 1
} io_mode;

extern volatile uint8_t* base_address;
extern io_mode current_io_mode;
extern io_mode current_inst_mode;
extern bool nor_flash_is_init(void);
void nor_flash_init(void);
void nor_flash_write(uint32_t address, uint8_t* data, size_t len);
void nor_flash_read(uint32_t address, uint8_t* data, size_t len);
void nor_flash_reset(void);