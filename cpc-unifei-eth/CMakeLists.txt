cmake_minimum_required(VERSION 3.15)

project(cpc-unifei-eth LANGUAGES C CXX ASM)

find_bsp(ID com.sysprogs.project_importers.stm32.cubemx SOURCE_PROJECT cpc-unifei-eth.ioc)

bsp_include_directories(. ../phasor-estimation-filters/phasorfilters/inc)
add_bsp_based_executable(
	NAME cpc-unifei-eth
	GENERATE_BIN
	GENERATE_MAP
	SOURCES
		${BSP_ROOT}/lwiperf.c
		${BSP_ROOT}/ads8686s.c
		${BSP_ROOT}/ads8686s.h
		${BSP_ROOT}/command_system.h
		${BSP_ROOT}/command_system.c
		${BSP_ROOT}/command_system_interface.h
		${BSP_ROOT}/command_system_interface.c
		${BSP_ROOT}/udp_server.h
		${BSP_ROOT}/udp_server.c
		${BSP_ROOT}/application.c
		${BSP_ROOT}/application.h
		${BSP_ROOT}/ads8686s_low_level.h
		${BSP_ROOT}/ads8686s_low_level.c
		${BSP_ROOT}/lua-5.4.7/src/lapi.h
		${BSP_ROOT}/lua-5.4.7/src/lauxlib.h
		${BSP_ROOT}/lua-5.4.7/src/lcode.h
		${BSP_ROOT}/lua-5.4.7/src/lctype.h
		${BSP_ROOT}/lua-5.4.7/src/ldebug.h
		${BSP_ROOT}/lua-5.4.7/src/ldo.h
		${BSP_ROOT}/lua-5.4.7/src/lfunc.h
		${BSP_ROOT}/lua-5.4.7/src/lgc.h
		${BSP_ROOT}/lua-5.4.7/src/ljumptab.h
		${BSP_ROOT}/lua-5.4.7/src/llex.h
		${BSP_ROOT}/lua-5.4.7/src/llimits.h
		${BSP_ROOT}/lua-5.4.7/src/lmem.h
		${BSP_ROOT}/lua-5.4.7/src/lobject.h
		${BSP_ROOT}/lua-5.4.7/src/lopcodes.h
		${BSP_ROOT}/lua-5.4.7/src/lopnames.h
		${BSP_ROOT}/lua-5.4.7/src/lparser.h
		${BSP_ROOT}/lua-5.4.7/src/lprefix.h
		${BSP_ROOT}/lua-5.4.7/src/lstate.h
		${BSP_ROOT}/lua-5.4.7/src/lstring.h
		${BSP_ROOT}/lua-5.4.7/src/ltable.h
		${BSP_ROOT}/lua-5.4.7/src/ltm.h
		${BSP_ROOT}/lua-5.4.7/src/lua.h
		${BSP_ROOT}/lua-5.4.7/src/lua.hpp
		${BSP_ROOT}/lua-5.4.7/src/luaconf.h
		${BSP_ROOT}/lua-5.4.7/src/lualib.h
		${BSP_ROOT}/lua-5.4.7/src/lundump.h
		${BSP_ROOT}/lua-5.4.7/src/lvm.h
		${BSP_ROOT}/lua-5.4.7/src/lzio.h
		${BSP_ROOT}/lua-5.4.7/src/lapi.c
		${BSP_ROOT}/lua-5.4.7/src/lauxlib.c
		${BSP_ROOT}/lua-5.4.7/src/lbaselib.c
		${BSP_ROOT}/lua-5.4.7/src/lcode.c
		${BSP_ROOT}/lua-5.4.7/src/lcorolib.c
		${BSP_ROOT}/lua-5.4.7/src/lctype.c
		${BSP_ROOT}/lua-5.4.7/src/ldblib.c
		${BSP_ROOT}/lua-5.4.7/src/ldebug.c
		${BSP_ROOT}/lua-5.4.7/src/ldo.c
		${BSP_ROOT}/lua-5.4.7/src/ldump.c
		${BSP_ROOT}/lua-5.4.7/src/lfunc.c
		${BSP_ROOT}/lua-5.4.7/src/lgc.c
		${BSP_ROOT}/lua-5.4.7/src/linit.c
		${BSP_ROOT}/lua-5.4.7/src/liolib.c
		${BSP_ROOT}/lua-5.4.7/src/llex.c
		${BSP_ROOT}/lua-5.4.7/src/lmathlib.c
		${BSP_ROOT}/lua-5.4.7/src/lmem.c
		${BSP_ROOT}/lua-5.4.7/src/loadlib.c
		${BSP_ROOT}/lua-5.4.7/src/lobject.c
		${BSP_ROOT}/lua-5.4.7/src/lopcodes.c
		${BSP_ROOT}/lua-5.4.7/src/loslib.c
		${BSP_ROOT}/lua-5.4.7/src/lparser.c
		${BSP_ROOT}/lua-5.4.7/src/lstate.c
		${BSP_ROOT}/lua-5.4.7/src/lstring.c
		${BSP_ROOT}/lua-5.4.7/src/lstrlib.c
		${BSP_ROOT}/lua-5.4.7/src/ltable.c
		${BSP_ROOT}/lua-5.4.7/src/ltablib.c
		${BSP_ROOT}/lua-5.4.7/src/ltm.c
		${BSP_ROOT}/lua-5.4.7/src/lua.c
		${BSP_ROOT}/lua-5.4.7/src/luac.c
		${BSP_ROOT}/lua-5.4.7/src/lundump.c
		${BSP_ROOT}/lua-5.4.7/src/lutf8lib.c
		${BSP_ROOT}/lua-5.4.7/src/lvm.c
		${BSP_ROOT}/lua-5.4.7/src/lzio.c)
target_include_directories(cpc-unifei-eth PRIVATE lua-5.4.7/src)
set_source_files_properties(${BSP_ROOT}/lua-5.4.7/src/lua.c ${BSP_ROOT}/lua-5.4.7/src/luac.c PROPERTIES HEADER_FILE_ONLY TRUE)
