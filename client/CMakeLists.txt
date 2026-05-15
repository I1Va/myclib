cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(client
    src/bot.c
    src/net.c
    src/protocol.c
    src/main.c
)

target_include_directories(client PRIVATE inc)

target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)
