cmake_minimum_required(VERSION 3.4)
project(codex_worker)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lzmq")

set(SOURCE_FILES src/main.cpp src/worker_config.cpp src/worker_config.hpp src/broker_connection.hpp)
add_executable(codex_worker ${SOURCE_FILES})