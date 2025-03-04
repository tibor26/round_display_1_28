set(_core_hwregister_list_file ../MM32F3270.svd)
set(_core_linker_script)

set(_core_includes
	../Drivers/MM32F327x/HAL_Lib/Inc
	../Drivers/MM32F327x/Include
	../Drivers/MM32F327x/startup/inc
	../Drivers/CMSIS/Core/Include
	../App/Inc
	../GUI)

set(_core_defines
	__MM3U1
	CLOCK=8000000)

set(_core_cflags
	"$<$<COMPILE_LANGUAGE:C>:-Wno-unused-parameter>"
	"$<$<COMPILE_LANGUAGE:C>:-ffunction-sections>"
	"$<$<COMPILE_LANGUAGE:C>:-fdata-sections>"
	"$<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>"
	"$<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>"
	"$<$<COMPILE_LANGUAGE:CXX>:-fdata-sections>"
	"$<$<COMPILE_LANGUAGE:ASM>:-Wno-unused-parameter>"
	"$<$<COMPILE_LANGUAGE:ASM>:-ffunction-sections>"
	"$<$<COMPILE_LANGUAGE:ASM>:-fdata-sections>")

set(_core_commonflags
	-mcpu=cortex-m3
	-mthumb
	-Wall
	-Wextra)

set(_core_ldflags
	--specs=nano.specs
	-lc
	-lnosys
	-Wl,--gc-sections
	-pedantic)

set(_core_ID)

