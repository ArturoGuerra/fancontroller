cmake_minimum_required(VERSION 3.28.1)

include(pico_sdk_import.cmake)
project(fancontroller C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)

pico_sdk_init()

add_compile_options(
  -Wall
  -Wno-format # int != int32_t as far as the compiler is concerned because gcc
              # has int32_t as long int
  -Wno-unused-function # we have some for the docs that aren't called
  -Wno-uninitialized)

add_subdirectory(pico-ssd1306)

add_executable(
  ${PROJECT_NAME} src/main.cpp src/fancontroller.cpp src/fanmonitor.cpp
                  src/speedsource.cpp src/display.cpp)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE pico_stdlib
          hardware_pio
          hardware_dma
          hardware_pwm
          hardware_adc
          hardware_i2c
          pico_ssd1306)

target_include_directories(${PROJECT_NAME}
                           PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})
