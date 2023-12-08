if(NOT ARDUINO_PATH)
    message(FATAL_ERROR "Arduino-specific variables are not set. \
                         Did you select the right toolchain file?")
endif()

# Arduino Core
file(GLOB CORE_FILES ${ARDUINO_CORE_PATH}/*.cpp ${ARDUINO_CORE_PATH}/*.c)
add_library(ArduinoCore STATIC ${CORE_FILES})

target_link_libraries(ArduinoCore PUBLIC ArduinoFlags)
target_compile_features(ArduinoCore PUBLIC cxx_std_11 c_std_11)
target_include_directories(ArduinoCore PUBLIC
    ${ARDUINO_RP2040_PATH}/include
    ${ARDUINO_CORE_PATH}
    ${ARDUINO_RP2040_PATH}/variants/${ARDUINO_VARIANT}
)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-m0plus")

target_compile_definitions(ArduinoCore PRIVATE
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

endif()

if(ARDUINO_USB)
    target_compile_definitions(ArduinoCore PUBLIC
        USB_VID=${ARDUINO_USB_VID}
        USB_PID=${ARDUINO_USB_PID}
        USB_MANUFACTURER=\"Unknown\"
        USB_PRODUCT=\"${ARDUINO_USB_PRODUCT}\"
    )
endif()
