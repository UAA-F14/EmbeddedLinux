cmake_minimum_required(VERSION 3.16)
project(SPI_Lib C)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.c)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE inc)
target_link_libraries(${PROJECT_NAME} PRIVATE gpiod)