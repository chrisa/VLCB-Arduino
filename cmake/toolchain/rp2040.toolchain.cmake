# Enter CMake cross-compilation mode
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR "cortex-m0plus")

# User settings with sensible defaults
set(ARDUINO_PATH "$ENV{HOME}/.arduino15/packages/rp2040" CACHE PATH
    "Path of the Arduino packages folder, e.g. ~/.arduino15/packages/rp2040.")
set(ARDUINO_USER_PATH "$ENV{HOME}/Arduino" CACHE PATH
    "Path of the Arduino user folder")
set(ARDUINO_CORE_VERSION "3.6.1" CACHE STRING
    "Version of Arduino-Pico core")
set(PQT_GCC_VERSION "2.1.0-a-d3d2e6b" CACHE STRING
    "Full version string of the GCC release shipped with the Arduino core.")
set(ARDUINO_VERSION "10815" CACHE STRING
    "Arduino IDE version (used for the macro with the same name)")

# Derived paths
set(ARDUINO_RP2040_PATH ${ARDUINO_PATH}/hardware/rp2040/${ARDUINO_CORE_VERSION})
set(ARDUINO_LIBS_PATH ${ARDUINO_RP2040_PATH}/libraries)
set(ARDUINO_CORE_PATH ${ARDUINO_RP2040_PATH}/cores/rp2040)
set(ARDUINO_PICO_SDK_PATH ${ARDUINO_RP2040_PATH}/pico-sdk)
set(ARDUINO_USER_LIBS_PATH ${ARDUINO_USER_PATH}/libraries)
set(ARDUINO_TOOLS_PATH ${ARDUINO_PATH}/tools/pqt-gcc/${PQT_GCC_VERSION}/bin)

# Toolchain paths
set(CMAKE_C_COMPILER ${ARDUINO_TOOLS_PATH}/arm-none-eabi-gcc CACHE FILEPATH
    "Path to arm-none-eabi-gcc" FORCE)
set(CMAKE_CXX_COMPILER ${ARDUINO_TOOLS_PATH}/arm-none-eabi-g++ CACHE FILEPATH
    "Path to arm-none-eabi-g++" FORCE)
set(CMAKE_OBJCOPY ${ARDUINO_TOOLS_PATH}/arm-none-eabi-objcopy CACHE FILEPATH
    "Path to arm-none-eabi-objcopy" FORCE)
set(CMAKE_SIZE ${ARDUINO_TOOLS_PATH}/arm-none-eabi-size CACHE FILEPATH
    "Path to arm-none-eabi-size" FORCE)

get_property(IS_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE)

foreach(LANG IN ITEMS C CXX ASM)
  if(IS_IN_TRY_COMPILE)
    set(CMAKE_${LANG}_LINK_FLAGS "${CMAKE_${LANG}_LINK_FLAGS} -nostdlib")
  endif()
endforeach()

# move to board file
set(ARDUINO_VARIANT "rpipico")
