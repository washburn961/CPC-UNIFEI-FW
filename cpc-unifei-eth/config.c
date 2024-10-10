#include "config.h"
#include "application.h"
#include "real_time.h"
#include "signal_processing.h"
#include "nor_flash.h"
#include <stdio.h>

bool config_is_set = false;
general_config* active_config;
general_config restored_config;

void config_save(void);

void config_set(general_config* configs)
{
	if (configs == NULL || configs->header.magic_number != CONFIG_MAGIC_NUMBER || configs->header.version != CONFIG_VERSION)
	{
		printf("Error: Invalid configuration provided.\n");
		return;
	}

	// Check if the configuration is the same as the current active configuration using UID
	if (config_is_set && active_config->header.uid == configs->header.uid)
	{
		printf("Configuration is already active. Skipping redundant operation.\n");
		return;  // No need to reconfigure or write to flash
	}
	
//	application_take();
//	real_time_take();
	
	signal_processing_channel_config(CHANNEL_0A, &(configs->analog.channel_0a));
	signal_processing_channel_config(CHANNEL_0B, &(configs->analog.channel_0b));
	signal_processing_channel_config(CHANNEL_1A, &(configs->analog.channel_1a));
	signal_processing_channel_config(CHANNEL_1B, &(configs->analog.channel_1b));
	signal_processing_channel_config(CHANNEL_2A, &(configs->analog.channel_2a));
	signal_processing_channel_config(CHANNEL_2B, &(configs->analog.channel_2b));
	signal_processing_channel_config(CHANNEL_3A, &(configs->analog.channel_3a));
	signal_processing_channel_config(CHANNEL_3B, &(configs->analog.channel_3b));
	signal_processing_channel_config(CHANNEL_4A, &(configs->analog.channel_4a));
	signal_processing_channel_config(CHANNEL_4B, &(configs->analog.channel_4b));
	signal_processing_channel_config(CHANNEL_5A, &(configs->analog.channel_5a));
	signal_processing_channel_config(CHANNEL_5B, &(configs->analog.channel_5b));
	signal_processing_channel_config(CHANNEL_6A, &(configs->analog.channel_6a));
	signal_processing_channel_config(CHANNEL_6B, &(configs->analog.channel_6b));
	signal_processing_channel_config(CHANNEL_7A, &(configs->analog.channel_7a));
	signal_processing_channel_config(CHANNEL_7B, &(configs->analog.channel_7b));
	
//	application_release();
//	real_time_release();

	// Update the global active configuration pointer
	active_config = configs;
	config_is_set = true;

	// Save the new configuration to flash memory
	config_save();
}

void config_get(general_config** out_configs)
{
	*out_configs = active_config;
}

bool config_restore(void)
{
	if (!nor_flash_is_init()) 
	{
		printf("Error: Flash not initialized.\n");
		return false;
	}

	// Read the entire configuration from flash into the restored_config buffer
	nor_flash_read(CONFIG_FLASH_SECTOR, (uint8_t*)(&restored_config), sizeof(general_config));

	// Validate the configuration in flash before applying
	if (restored_config.header.magic_number != CONFIG_MAGIC_NUMBER || restored_config.header.version != CONFIG_VERSION)
	{
		printf("Error: Invalid or incompatible configuration in flash.\n");
		return false;
	}

	// Check if the UID in flash matches the current active configuration's UID
	if (config_is_set && restored_config.header.uid == active_config->header.uid)
	{
		printf("Configuration in flash is already active (UID: %lu). Skipping restore.\n", restored_config.header.uid);
		return true;
	}

	// Set the restored configuration as the active configuration
	config_set(&restored_config);
	printf("Configuration (UID: %lu) restored from flash and applied.\n", restored_config.header.uid);
	return true;
}

void config_save(void)
{
	if (!nor_flash_is_init()) 
	{
		printf("Error: Flash not initialized. Cannot save configuration.\n");
		return;
	}

	// Check if the active configuration is valid
	if (active_config == NULL)
	{
		printf("Error: No active configuration to save.\n");
		return;
	}

	// Buffer to hold the flash header
	general_config_header flash_header = { 0 };

	// Read only the header from flash memory
	nor_flash_read(CONFIG_FLASH_SECTOR, (uint8_t*)(&flash_header), sizeof(general_config_header));

	// Check if the UID and version in flash match the current configuration
	if (flash_header.magic_number == CONFIG_MAGIC_NUMBER &&
	    flash_header.uid == active_config->header.uid &&
	    flash_header.version == active_config->header.version)
	{
		printf("Configuration in flash is already up-to-date (UID: %lu).\n", flash_header.uid);
		return;  // Skip writing if the configuration is the same
	}

	// If the configuration differs, write the current configuration to flash memory
	nor_flash_write(CONFIG_FLASH_SECTOR, (uint8_t*)(active_config), sizeof(general_config));

	printf("Configuration (UID: %lu) saved to flash.\n", active_config->header.uid);
}