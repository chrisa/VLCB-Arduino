if(NOT ARDUINO_PATH)
    message(FATAL_ERROR "Arduino-specific variables are not set. \
                         Did you select the right toolchain file?")
endif()

file(GLOB_RECURSE LIBS_FILES ${ARDUINO_LIBS_PATH}/*.cpp ${ARDUINO_LIBS_PATH}/*.c)
list(FILTER LIBS_FILES EXCLUDE REGEX ".*(Bluetooth|BT|BLE|Adafruit|ESP8266SdFat|FreeRTOS).*")
list(FILTER LIBS_FILES EXCLUDE REGEX "LittleFS") # needs more complex build
list(FILTER LIBS_FILES EXCLUDE REGEX "PDM") # PDM ?

add_library(ArduinoLibs STATIC ${LIBS_FILES})
target_link_libraries(ArduinoLibs PUBLIC ArduinoFlags)
target_link_libraries(ArduinoLibs PUBLIC ArduinoCore)

target_compile_features(ArduinoLibs PUBLIC cxx_std_11 c_std_11)

file(GLOB LIBRARIES ${ARDUINO_LIBS_PATH}/*)
list(TRANSFORM LIBRARIES APPEND "/src")
target_include_directories(ArduinoLibs PUBLIC ${LIBRARIES})

# Pico-specific
#target_compile_options(ArduinoLibs PRIVATE
#    "$<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>"
#)
target_compile_definitions(ArduinoLibs PRIVATE
#    "ENABLE_CLASSIC"
    "CFG_TUSB_MCU=OPT_MCU_RP2040"
    "USBD_PID=0x000a"
    "USBD_VID=0x2e8a"
    "USBD_MAX_POWER_MA=250"
    "USB_MANUFACTURER=\"Raspberry Pi\""
    "USB_PRODUCT=\"Pico\""
    "ARM_MATH_CM0_FAMILY"
    "ARM_MATH_CM0_PLUS"
    "F_CPU=133000000L"
    "PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1"
    "CYW43_LWIP=1"
    "LWIP_IPV6=0"
    "LWIP_IPV4=1"
    "LWIP_IGMP=1"
    "LWIP_CHECKSUM_CTRL_PER_NETIF=1"
    "WIFICC=CYW43_COUNTRY_WORLDWIDE"
)
