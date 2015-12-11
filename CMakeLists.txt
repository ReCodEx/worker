cmake_minimum_required(VERSION 3.3)
project(codex_worker)

# ZeroMQ C++ bindings headers
include_directories(AFTER, vendor/cppzmq)

# Use C++11 and link with ZeroMQ support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lzmq")

# The worker executable
set(SOURCE_FILES
        src/main.cpp
        src/worker_config.cpp
        src/worker_config.hpp
        src/broker_connection.hpp
)

add_executable(codex_worker ${SOURCE_FILES})

# Include Google Test libraries and then our very own unit tests
add_subdirectory(vendor/googletest)
add_subdirectory(tests)