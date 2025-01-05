cmake_minimum_required(VERSION 3.22)

project(snake C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_executable(client src/main_client.c src/ipc.c)
add_executable(server src/main_server.c src/ipc.c)
