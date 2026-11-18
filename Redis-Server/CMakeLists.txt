cmake_minimum_required(VERSION 3.16)

project(RedisServer LANGUAGES CXX)

# ---- C++ settings --------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug if no build type was given
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

# ---- Executable ----------------------------------------------------------
add_executable(redis-server
    src/main.cpp
    src/RedisServer.cpp
    src/RedisCommandHandler.cpp
)

target_include_directories(redis-server PRIVATE include)

target_link_libraries(redis-server PRIVATE Threads::Threads)

target_compile_options(redis-server PRIVATE -Wall -Wextra)