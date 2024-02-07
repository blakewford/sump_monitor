cmake_minimum_required(VERSION 3.22.1)
project("sump_monitor")
add_executable(sump_monitor ./src/main.cpp ./src/mqtt.cpp ./src/bmp.cpp)
target_link_libraries(sump_monitor)
