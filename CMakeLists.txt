cmake_minimum_required(VERSION 3.2)
project(codex_worker)

# ZeroMQ C++ bindings headers
include_directories(AFTER, vendor/cppzmq)

# Use C++11 and link with ZeroMQ support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "-lzmq")

# The worker executable
set(SOURCE_FILES
        src/main.cpp
        src/worker_config.cpp
        src/worker_config.hpp
        src/broker_connection.hpp
        src/connection_proxy.hpp)

add_executable(codex_worker ${SOURCE_FILES})
target_link_libraries(codex_worker yaml-cpp)

# Include Google Test libraries and then our very own unit tests
add_subdirectory(vendor/googletest)
add_subdirectory(tests)
