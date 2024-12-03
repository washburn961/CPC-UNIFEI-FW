#pragma once

#include <stdio.h>

// Define DEBUG_MODE here or in your project's configuration settings
#define DEBUG_MODE 1  // Set to 0 to disable debug prints

// Macro for debug printing based on DEBUG_MODE
#if DEBUG_MODE
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...)  printf("[INFO] " fmt, ##__VA_ARGS__)
#define DEBUG_WARN(fmt, ...)  printf("[WARN] " fmt, ##__VA_ARGS__)
#define DEBUG_ERROR(fmt, ...) printf("[ERROR] " fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) ((void)0)
#define DEBUG_INFO(fmt, ...)  ((void)0)
#define DEBUG_WARN(fmt, ...)  ((void)0)
#define DEBUG_ERROR(fmt, ...) ((void)0)
#endif
