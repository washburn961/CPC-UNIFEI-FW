#include "is25lp064a.h"
#include "octospi.h"

void EnterQPIMode(void);
void WriteEnable(void);
void WriteEnable_OneLine(void);
void WriteReg_OneLine(uint8_t instruction, uint8_t data);

void IS25LP064A_Init(void)
{
	uint8_t statusreg = 0;
	WriteReg_OneLine(0x01, 0b00000000);
	WriteReg_OneLine(0x01, 0b01000000);
	EnterQPIMode();
	IS25LP064A_ReadReg(0x05, &statusreg);
	IS25LP064A_WriteReg(0xC0, 0b11110000);
	HAL_Delay(1);
}

void IS25LP064A_Write(uint32_t address, uint8_t *data, uint32_t length)
{
	OSPI_RegularCmdTypeDef sCommand;

	// Step 1: Configure the command for Page Program (0x02 in QPI mode)
	sCommand.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId             = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction         = 0x02;  // Page Program command
	sCommand.InstructionMode     = HAL_OSPI_INSTRUCTION_4_LINES;
	sCommand.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.Address             = address;
	sCommand.AddressMode         = HAL_OSPI_ADDRESS_4_LINES;
	sCommand.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
	sCommand.DataMode            = HAL_OSPI_DATA_4_LINES;
	sCommand.NbData              = length;
	sCommand.DummyCycles         = 0;
	sCommand.DQSMode             = HAL_OSPI_DQS_DISABLE;
	sCommand.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;

	// Step 2: Write Enable before programming
	WriteEnable();

	// Step 3: Send the Page Program command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}

	// Step 4: Transmit the data
	if (HAL_OSPI_Transmit(&hospi1, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}

void IS25LP064A_EraseSector(uint32_t address)
{
	
}

void IS25LP064A_Read(uint32_t address, uint8_t *out_data, uint32_t length)
{
	OSPI_RegularCmdTypeDef sCommand;

	// Step 1: Configure the command for Fast Read Quad I/O (0xEB in QPI mode)
	sCommand.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId             = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction         = 0xEB;  // Fast Read Quad I/O command
	sCommand.InstructionMode     = HAL_OSPI_INSTRUCTION_4_LINES;
	sCommand.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.Address             = address;
	sCommand.AddressMode         = HAL_OSPI_ADDRESS_4_LINES;
	sCommand.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
	sCommand.AlternateBytes      = 0x00;  // Mode bits in QPI mode
	sCommand.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_4_LINES;
	sCommand.AlternateBytesSize  = HAL_OSPI_ALTERNATE_BYTES_8_BITS;
	sCommand.DataMode            = HAL_OSPI_DATA_4_LINES;
	sCommand.NbData              = length;
	sCommand.DummyCycles         = 8;  // As per IS25LP064A datasheet for 0xEB command
	sCommand.DQSMode             = HAL_OSPI_DQS_DISABLE;
	sCommand.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;

	// Step 2: Send the Fast Read command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}

	// Step 3: Receive the data
	if (HAL_OSPI_Receive(&hospi1, out_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
	}
}


void IS25LP064A_WriteReg(uint8_t instruction, uint8_t data)
{
	OSPI_RegularCmdTypeDef sCommand = { 0 };

	/* Initialize the command structure */
	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = instruction;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_4_LINES;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;
	
	WriteEnable();

	// Send the command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors
	}

	// Transmit the data
	if (HAL_OSPI_Transmit(&hospi1, &data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors
	}
}

void IS25LP064A_ReadReg(uint8_t instruction, uint8_t *out_data)
{
	OSPI_RegularCmdTypeDef sCommand = { 0 };

	/* Initialize the command structure */
	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = instruction;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_4_LINES;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;

	// Send the command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors
	}

	// Receive the data
	if (HAL_OSPI_Receive(&hospi1, out_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors
	}
}

void EnterQPIMode(void)
{
	OSPI_RegularCmdTypeDef sCommand = { 0 };

	/* Initialize the command structure */
	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = 0x35;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	/* Send the command */
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors as needed
	}
}

void WriteEnable(void)
{
	OSPI_RegularCmdTypeDef sCommand = { 0 };
	OSPI_AutoPollingTypeDef sConfig = { 0 };

	// Step 1: Configure the command for Write Enable
	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = 0x06;  // Write Enable command (0x06)
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	// Step 2: Send the Write Enable command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors as needed
	}

	// Step 3: Reconfigure the command to read the status register
	sCommand.Instruction = 0x05;  // Read Status Register command (0x05)
	sCommand.DataMode = HAL_OSPI_DATA_4_LINES;
	sCommand.NbData = 1;

	// Step 4: Send the Read Status Register command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors as needed
	}
	
	HAL_Delay(1);
	
	uint8_t reg = 0;
	
	HAL_OSPI_Receive(&hospi1, &reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
	
	HAL_Delay(1);
	
//	// Step 5: Configure the auto-polling for the WEL bit to become 1
//	sConfig.Match = 0x02;  // Match when WEL bit is 1 (0x02)
//	sConfig.Mask = 0x02;   // Mask for the WEL bit
//	sConfig.MatchMode = HAL_OSPI_MATCH_MODE_AND;
//	sConfig.Interval = 0x10;  // Polling interval (adjust as needed)
//	sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
//
//	// Step 6: Start auto-polling to wait for the WEL bit to be set
//	if (HAL_OSPI_AutoPolling(&hospi1, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//	{
//		//Error_Handler();  // Handle errors as needed
//	}
}

void WriteEnable_OneLine(void)
{
	OSPI_RegularCmdTypeDef sCommand = { 0 };
	OSPI_AutoPollingTypeDef sConfig = { 0 };

	// Step 1: Configure the command for Write Enable
	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = 0x06;  // Write Enable command (0x06)
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	// Step 2: Send the Write Enable command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors as needed
	}

	// Step 3: Reconfigure the command to read the status register
	sCommand.Instruction = 0x05;  // Read Status Register command (0x05)
	sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
	sCommand.NbData = 1;
	
	HAL_Delay(1);

	// Step 4: Send the Read Status Register command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors as needed
	}
	
	uint8_t reg = 0;
	if (HAL_OSPI_Receive(&hospi1, &reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors as needed
	}
	
	HAL_Delay(1);

//	// Step 5: Configure the auto-polling for the WEL bit to become 1
//	sConfig.Match = 0x02;  // Match when WEL bit is 1 (0x02)
//	sConfig.Mask = 0x02;   // Mask for the WEL bit
//	sConfig.MatchMode = HAL_OSPI_MATCH_MODE_AND;
//	sConfig.Interval = 0x10;  // Polling interval (adjust as needed)
//	sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
//
//	// Step 6: Start auto-polling to wait for the WEL bit to be set
//	if (HAL_OSPI_AutoPolling(&hospi1, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//	{
//		Error_Handler();  // Handle errors as needed
//	}
}

void WriteReg_OneLine(uint8_t instruction, uint8_t data)
{
	OSPI_RegularCmdTypeDef sCommand = { 0 };

	/* Initialize the command structure */
	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = instruction;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;
	
	WriteEnable_OneLine();

	// Send the command
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors
	}

	// Transmit the data
	if (HAL_OSPI_Transmit(&hospi1, &data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();  // Handle errors
	}
}