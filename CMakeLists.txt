# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Set project name for convenience
set(PNAME JBC200W)

# using the ssd1309 Graphics Driver
set(DispDriver ssd1309)

# pathing prefixes for additional code modules
set(ModulePath        "${CMAKE_CURRENT_LIST_DIR}/modules")
set(ModulePicoDrivers "${ModulePath}/picoDrivers")
set(ModulePicoDisp    "${ModulePicoDrivers}/display")
set(ModulePicoKey     "${ModulePicoDrivers}/keyboard")
set(ModuleDispDriver  "${ModulePicoDisp}/${DispDriver}")

# Cannot use a variable here for the project name as the stupid Pico parser does not resolve it..
# it just "greps" this line to get the literal text. Hence you get problems like"
# cannot launch project ${FOO}.elf in the debugger.
project(JBC200W C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Assemble source files
set(DisplayBaseFiles
    ${ModulePicoDisp}/displayBSP.c
    ${ModulePicoDisp}/common/cpyutils.c
    ${ModulePicoDisp}/common/led_overlay.c
    ${ModulePicoDisp}/common/linegfx.c
    ${ModulePicoDisp}/common/textgfx.c
)

set(KeypadFiles
    ${ModulePicoKey}/keygpio/keygpio.c
)

# display driver source files
set(DispDrvrFiles
    ${ModuleDispDriver}/${DispDriver}_driver.c
)

# local program source files
set(localFiles
    ${PNAME}.c
    jbc_util.c
    display.c
    keypad.c
    operations.c
) 

# Add executable. Default name is the project name, version 0.1
add_executable(${PNAME} 
    ${localFiles}
    ${DisplayBaseFiles}
    ${KeypadFiles}
    ${DispDrvrFiles}
)

pico_set_program_name(${PNAME} "${PNAME}")
pico_set_program_version(${PNAME} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PNAME} 1)
pico_enable_stdio_usb(${PNAME} 0)

# Add the standard library to the build
target_link_libraries(${PNAME}
    pico_stdlib
    hardware_spi
    pico_rand
    hardware_timer
)

# Add the standard include files to the build
target_include_directories(${PNAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${ModulePicoDisp}
    ${ModulePicoDisp}/include
    ${ModulePicoKey}/include
    ${ModulePicoKey}/keygpio/
    ${ModuleDispDriver}
)

# force compiler to make debuggable code (un optimized)
SET(CMAKE_C_FLAGS_RELEASE "-g -O0")

pico_add_extra_outputs(${PNAME})

