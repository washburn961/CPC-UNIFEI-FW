#include "lua_interface.h"
#include "gpio.h"

lua_State *L;

// Function to toggle the LED (connected to GPIO pin)
int lua_toggle_led(lua_State *L)
{
	HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
	return 0;  // Returning 0 because we're not returning any value to Lua
}

static const struct luaL_Reg mylib[] = {
	{ "toggle_led", lua_toggle_led }, // Register the toggle function
	{ NULL, NULL }  // Sentinel to indicate the end of the list
};

void lua_init(void)
{
	L = luaL_newstate(); // Create a new Lua state with a custom allocator
	luaL_openlibs(L); // Load Lua standard libraries (this includes os and io)

    // Push nil to remove the os library
//	lua_pushnil(L);
//	lua_setglobal(L, "os");
//
//	// Push nil to remove the io library
//	lua_pushnil(L);
//	lua_setglobal(L, "io");

	// Register our custom C functions to Lua
	luaL_newlib(L, mylib);
	lua_setglobal(L, "gpio"); // This makes the functions accessible as gpio.toggle_led() in Lua scripts
}

// Minimal definition of struct timeval for embedded use
struct timeval {
	long tv_sec; // seconds
	long tv_usec; // microseconds
};

// The _gettimeofday function
int _gettimeofday(struct timeval *tv, void *tzvp)
{
	if (tv) {
		tv->tv_sec = 0; // Set seconds to zero (or other value if needed)
		tv->tv_usec = 0; // Set microseconds to zero
	}
	return 0;  // Return 0 to indicate success
}