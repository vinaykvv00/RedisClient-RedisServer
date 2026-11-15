cmake_minimum_required(VERSION 3.16)

project(RedisClient LANGUAGES CXX)

# ---- C++ settings --------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug if no build type was given
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---- Executable ----------------------------------------------------------
add_executable(redis-client
    main.cpp
    src/CLI.cpp
    src/RedisClient.cpp
    src/CommandHandler.cpp
)

# Headers live in include/ (CLI.hpp, RedisClient.hpp)
target_include_directories(redis-client PRIVATE include)

# Warnings only; no extra libraries needed (Berkeley sockets are in libc)
target_compile_options(redis-client PRIVATE -Wall -Wextra)
