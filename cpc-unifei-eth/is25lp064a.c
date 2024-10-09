#include "is25lp064a.h"
#include "octospi.h"
#include "stdio.h"
#include "debug.h"

void write_enable(io_mode mode);

void is25lp064a_init(void)
{
	is25lp064a_reset(spi);
}
void is25lp064a_reset(io_mode mode)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_WREN;
	command.InstructionMode = mode == qspi ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = HAL_OSPI_DATA_NONE;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	
	HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
	
	command.Instruction = INST_RST;

	HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}

void write_enable(io_mode mode)
{
	OSPI_RegularCmdTypeDef command = { 0 };
	OSPI_AutoPollingTypeDef auto_polling_config = { 0 };
	uint8_t status = 0x0;
	
	auto_polling_config.Match = STSREG_WEL; // Match value: WEL bit set to 1
	auto_polling_config.Mask = STSREG_WEL; // Mask value: Only check WEL bit
	auto_polling_config.MatchMode = HAL_OSPI_MATCH_MODE_AND; // Require exact match of WEL bit
	auto_polling_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE; // Stop when match is found
	auto_polling_config.Interval = 0x10; // Interval between polling reads (adjust as necessary)
	
	command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	command.FlashId = HAL_OSPI_FLASH_ID_1;
	command.Instruction = INST_WREN;
	command.InstructionMode = mode == qspi ? HAL_OSPI_INSTRUCTION_4_LINES : HAL_OSPI_INSTRUCTION_1_LINE;
	command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	command.AddressMode = HAL_OSPI_ADDRESS_NONE;
	command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	command.DataMode = HAL_OSPI_DATA_NONE;
	command.DummyCycles = 0;
	command.DQSMode = HAL_OSPI_DQS_DISABLE;
	command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	
	HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
	
	command.Instruction = INST_RDSR;
	command.DataMode = mode == qspi ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_1_LINE;
	command.NbData = 1;
	
	HAL_OSPI_Command(&hospi1, &command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
	HAL_OSPI_AutoPolling(&hospi1, &auto_polling_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}