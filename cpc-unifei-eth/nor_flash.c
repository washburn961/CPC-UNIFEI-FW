#include "nor_flash.h"
#include "octospi.h"
#include <stdio.h>
#include "debug.h"

volatile uint8_t* base_address;
io_mode current_io_mode = SPI;
io_mode current_inst_mode = SPI;
uint8_t fast_read_dummy_cycles = 0;

void nor_flash_write_enable(void);
void poll_for_write_enable(void);
void poll_for_write_in_progress(void);
void configure_fast_read_dummy_cycles(void);
void memory_mapped_mode(void);
void enter_qpi(void);

void nor_flash_init(void)
{
	nor_flash_reset();
	enter_qpi();
	configure_fast_read_dummy_cycles();
    
	base_address = (uint8_t*)OCTOSPI1_BASE;
	DEBUG_INFO("%s", "EXTERNAL FLASH: INIT DONE\r\n");
}

void nor_flash_reset(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
    
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_WREN;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = HAL_OSPI_DATA_NONE;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	command.Instruction = INST_RST;

	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}

void nor_flash_write_enable(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
    
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_WREN;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = HAL_OSPI_DATA_NONE;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	poll_for_write_enable();
}

void poll_for_write_enable(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	OSPI_AutoPollingTypeDef auto_polling_config = { 0 };
    
	auto_polling_config.Match = STSREG_WEL;
	auto_polling_config.Mask = STSREG_WEL;
	auto_polling_config.MatchMode = HAL_OSPI_MATCH_MODE_AND;
	auto_polling_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
	auto_polling_config.Interval = 0x10;
    
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_RDSR;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.NbData = 1;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_OSPI_AutoPolling(&hospi1, &auto_polling_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}

void poll_for_write_in_progress(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	OSPI_AutoPollingTypeDef auto_polling_config = { 0 };
    
	auto_polling_config.Match = 0x0;
	auto_polling_config.Mask = STSREG_WIP;
	auto_polling_config.MatchMode = HAL_OSPI_MATCH_MODE_AND;
	auto_polling_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
	auto_polling_config.Interval = 0x10;
    
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_RDSR;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.NbData = 1;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_OSPI_AutoPolling(&hospi1, &auto_polling_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}

void enter_qpi(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	uint8_t status_reg = 0;
    
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_QPIEN;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = HAL_OSPI_DATA_NONE;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	current_inst_mode = QSPI;
	current_io_mode = QSPI;
    
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.Instruction = INST_RDSR;
	command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.NbData = 1;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_OSPI_Receive(&hospi1, &status_reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	SET_BIT(status_reg, STSREG_QE);
    
	command.Instruction = INST_WRSR;
    
	nor_flash_write_enable();
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_OSPI_Transmit(&hospi1, &status_reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	poll_for_write_in_progress();
    
	command.Instruction = INST_RDSR;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_OSPI_Receive(&hospi1, &status_reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}

void configure_fast_read_dummy_cycles(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	uint8_t read_reg = 0b11100000;
    
	SET_BIT(read_reg, READREG_DUMMY_CYCLES_P4);
	SET_BIT(read_reg, READREG_DUMMY_CYCLES_P3);
    
	fast_read_dummy_cycles = 10;
    
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_SRP;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.NbData = 1;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_OSPI_Transmit(&hospi1, &read_reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}

void memory_mapped_mode(void)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	OSPI_MemoryMappedTypeDef memory_mapped_config = { 0 };
    
	command.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_PP;
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = current_io_mode == QSPI ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_1_LINE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.NbData = 1;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	command.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
	command.Instruction = INST_FRD;
	command.DummyCycles = fast_read_dummy_cycles;
    
	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
    
	memory_mapped_config.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_ENABLE;
	memory_mapped_config.TimeOutPeriod = 0x50;
    
	if (HAL_OSPI_MemoryMapped(&hospi1, &memory_mapped_config) != HAL_OK)
	{
		Error_Handler();
	}
}

void nor_flash_write(uint32_t address, uint8_t* data, size_t len)
{
	// Calculate the sector of the starting address
	uint32_t start_sector = address / SECTOR_SIZE;
	uint32_t end_sector = (address + len - 1) / SECTOR_SIZE;

	// Ensure data does not span across multiple sectors
	if (start_sector != end_sector) {
		DEBUG_ERROR("Write operation spans multiple sectors, operation aborted.\r\n");
		return;
	}

	// Call Write Enable function before any erase or write operation
	nor_flash_write_enable();

	// Erase the sector before writing
	OSPI_RegularCmdTypeDef command = { 0 };
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_SER; // SER (Sector Erase) instruction
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = current_io_mode == QSPI ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_1_LINE;
	command.AddressSize = HAL_OSPI_ADDRESS_24_BITS; // 3-byte address
	command.Address = start_sector * SECTOR_SIZE;
	command.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = HAL_OSPI_DATA_NONE;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}

	// Poll until erase is complete
	poll_for_write_in_progress();

	// Write the data in pages of 256 bytes
	size_t bytes_written = 0;
	while (bytes_written < len)
	{
		size_t bytes_to_write = len - bytes_written;
		if (bytes_to_write > PAGE_SIZE) {
			bytes_to_write = PAGE_SIZE;
		}

		// Call Write Enable function before each page program
		nor_flash_write_enable();

		// Set up Page Program (PP) command
		command.Instruction = INST_PP; // Page Program
		command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
		command.NbData = bytes_to_write;

		// Set the address for the current page write
		command.Address = address + bytes_written;

		// Send command
		if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			Error_Handler();
		}

		// Transmit the data to flash
		if (HAL_OSPI_Transmit(&hospi1, data + bytes_written, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			Error_Handler();
		}

		// Poll until write is complete
		poll_for_write_in_progress();

		// Update the number of bytes written
		bytes_written += bytes_to_write;
	}

	DEBUG_INFO("Write operation successful: %d bytes written to address 0x%08X\r\n", len, address);
}

// Read data from external NOR flash memory
void nor_flash_read(uint32_t address, uint8_t* data, size_t len)
{
	// Set up Fast Read (FRD) command
	OSPI_RegularCmdTypeDef command = { 0 };
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_FRD; // Fast Read
	command.InstructionMode = current_inst_mode == QSPI ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = current_io_mode == QSPI ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_1_LINE;
	command.AddressSize = HAL_OSPI_ADDRESS_24_BITS; // 3-byte address
	command.Address = address;
	command.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = current_io_mode == QSPI ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.DummyCycles = fast_read_dummy_cycles;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	size_t bytes_read = 0;
	while (bytes_read < len)
	{
		// Calculate the remaining bytes to read
		size_t bytes_to_read = len - bytes_read;
		if (bytes_to_read > PAGE_SIZE) {
			bytes_to_read = PAGE_SIZE;
		}

		// Update the address for the current page read
		command.Address = address + bytes_read;

		// Update the number of bytes to read for the current operation
		command.NbData = bytes_to_read;

		// Send command
		if (HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			Error_Handler();
		}

		// Receive data
		if (HAL_OSPI_Receive(&hospi1, data + bytes_read, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			Error_Handler();
		}

		// Update the number of bytes read
		bytes_read += bytes_to_read;
	}

	DEBUG_INFO("Read operation successful: %d bytes read from address 0x%08X\r\n", len, address);
}
