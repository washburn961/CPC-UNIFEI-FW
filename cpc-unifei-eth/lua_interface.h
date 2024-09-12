#pragma once

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern lua_State *L;

void lua_init(void);