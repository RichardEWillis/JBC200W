Main body of code for the JBC 200W Iron Controller Board firmware.

PCB: 200W-JBC
Ver. v3.1 (Mainboard, Eval)

# Controller
Raspberry Pi Pico RP2040 (circa 2020) 40-pin SoM

# Build Environment
VSCode (Linux) with "Raspberry Pi Pico Project" extension.

## Build Setup
VSCode should see this as an established Pico code project. 
This assumes the PICO SDK is installed at: ${HOME}/.pico-sdk/
CMake and Ninja are required for building.

First setup the submodules by running:
`git submodules update --init --recursive`

Check the CMakeLists.txt to see if Release is using the compiler debug flags. To use normal optimization, comment out line: `SET(CMAKE_C_FLAGS_RELEASE "-g -O0")`

Next, in the Pico Code extension, invoke [Configure CMake] followed by [Compile Project]

Installing to target by Flash or debug it. If debugging you may want to re-enable the release debug optimizations (see above).

