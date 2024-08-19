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
		${BSP_ROOT}/is25lp064a.c
		${BSP_ROOT}/is25lp064a.h
		${BSP_ROOT}/ads8686s.c
		${BSP_ROOT}/ads8686s.h
		${BSP_ROOT}/ads8686s_interface.c
		${BSP_ROOT}/ads8686s_interface.h)

add_subdirectory(../phasor-estimation-filters/phasorfilters phasorfilters)
