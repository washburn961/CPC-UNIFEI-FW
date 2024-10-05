cmake_minimum_required(VERSION 3.15)

project(cpc-unifei-eth LANGUAGES C CXX ASM)

find_bsp(ID com.sysprogs.project_importers.stm32.cubemx SOURCE_PROJECT cpc-unifei-eth.ioc)

bsp_include_directories(. ../phasor-estimation-filters/phasorfilters/inc phasorfilters/inc power-system-protection/src/include)
add_bsp_based_executable(
	NAME cpc-unifei-eth
	GENERATE_BIN
	GENERATE_MAP
	SOURCES
		${BSP_ROOT}/lwiperf.c
		${BSP_ROOT}/ads8686s.c
		${BSP_ROOT}/ads8686s.h
		${BSP_ROOT}/udp_server.h
		${BSP_ROOT}/udp_server.c
		${BSP_ROOT}/application.c
		${BSP_ROOT}/application.h
		${BSP_ROOT}/ads8686s_low_level.h
		${BSP_ROOT}/ads8686s_low_level.c
		${BSP_ROOT}/phasorfilters/inc/cosine_filter.h
		${BSP_ROOT}/phasorfilters/inc/fourier_transform.h
		${BSP_ROOT}/phasorfilters/inc/half_cycle_fourier_transform.h
		${BSP_ROOT}/phasorfilters/inc/phasor_filters_conf.h
		${BSP_ROOT}/phasorfilters/inc/ring_buffer.h
		${BSP_ROOT}/phasorfilters/inc/sine_filter.h
		${BSP_ROOT}/phasorfilters/src/cosine_filter.c
		${BSP_ROOT}/phasorfilters/src/fourier_transform.c
		${BSP_ROOT}/phasorfilters/src/half_cycle_fourier_transform.c
		${BSP_ROOT}/phasorfilters/src/ring_buffer.c
		${BSP_ROOT}/phasorfilters/src/sine_filter.c
		${BSP_ROOT}/goose_frame.h
		${BSP_ROOT}/iec61850/ber.h
		${BSP_ROOT}/iec61850/goose.h
		${BSP_ROOT}/iec61850/goose_publisher.h
		${BSP_ROOT}/iec61850/iec_time.h
		${BSP_ROOT}/iec61850/semaphore_interface.h
		${BSP_ROOT}/iec61850/ber.c
		${BSP_ROOT}/iec61850/goose.c
		${BSP_ROOT}/iec61850/goose_publisher.c
		${BSP_ROOT}/sempahore_interface.c
		${BSP_ROOT}/power-system-protection/src/ANSI50.c
		${BSP_ROOT}/power-system-protection/src/ANSI51.c
		${BSP_ROOT}/power-system-protection/src/include/ANSI50.h
		${BSP_ROOT}/power-system-protection/src/include/ANSI51.h
		${BSP_ROOT}/power-system-protection/src/include/cpc_complex.h
		${BSP_ROOT}/power-system-protection/src/cpc_complex.c
		${BSP_ROOT}/power-system-protection/src/ANSI87B.c
		${BSP_ROOT}/power-system-protection/src/include/ANSI87B.h)
target_include_directories(cpc-unifei-eth PRIVATE lua-5.4.7/src phasorfilters/inc ../../../iec61850/iec61850 iec61850 iec61850 power-system-protection/src/include src/include src/include src/include)
set_source_files_properties(${BSP_ROOT}/src/include/ANSI67.h ${BSP_ROOT}/src/include/ANSI21.h ${BSP_ROOT}/src/include/ANSI50.h ${BSP_ROOT}/goose.h ${BSP_ROOT}/goose.c ${BSP_ROOT}/lua-5.4.7/src/lua.c ${BSP_ROOT}/lua-5.4.7/src/luac.c PROPERTIES HEADER_FILE_ONLY TRUE)
