cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)


project(pico1 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(external_lib)


link_directories(libmicroros)


add_executable(pico1
    pico1.cpp

)
target_link_libraries(pico1
    pico_stdlib
    microros
    kinematics
    pico_uart_transports
    motors
    rpm
    servo
    imu
)

target_include_directories(pico1 PUBLIC
    libmicroros/include

)



SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

# Configure Pico
pico_enable_stdio_usb(pico1 1)
pico_enable_stdio_uart(pico1 0)
add_compile_definitions(PICO_UART_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_DEFAULT_CRLF=0)


# Generate UF2
pico_add_extra_outputs(pico1)
