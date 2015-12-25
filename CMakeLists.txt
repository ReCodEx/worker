cmake_minimum_required(VERSION 2.8)
project(codex_worker)


set(EXEC_NAME ${PROJECT_NAME})
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable yaml-cpp testing and parse tools")
set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib.")


# MS Visual C++ specific bindings
if(MSVC)
	set(Boost_USE_STATIC_LIBS ON)
	find_package(Boost 1.57.0 REQUIRED COMPONENTS filesystem system)
	include_directories(${Boost_INCLUDE_DIRS})
	
	find_package(CURL REQUIRED)
	include_directories(${CURL_INCLUDE_DIRS})
	
	# ZeroMQ on Windows...
	find_path(ZEROMQ_INCLUDE NAMES zmq.h PATHS "C:\\deps\\libzmq\\include")
	find_library(ZEROMQ_LIB NAMES libzmq PATHS "C:\\deps\\libzmq\\lib")
	
	if(ZEROMQ_LIB)
		include_directories(${ZEROMQ_INCLUDE})
	else()
		message(FATAL_ERRORO "Fail to load ZeroMQ!")
	endif()
endif()


# ZeroMQ C++ bindings headers
include_directories(AFTER, vendor/cppzmq)

# Spdlog logging library
include_directories(AFTER, vendor/spdlog/include)

# Yaml-cpp
include_directories(AFTER, vendor/yaml-cpp/include)

# Use C++11
if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# The worker executable
set(SOURCE_FILES
	src/main.cpp
	src/worker_config.cpp
	src/worker_config.hpp
	src/broker_connection.hpp
	src/connection_proxy.hpp
	src/fileman/file_manager_base.h
	src/fileman/cache_manager.h
	src/fileman/http_manager.h
	src/fileman/file_manager.h
	src/fileman/cache_manager.cpp
	src/fileman/http_manager.cpp
	src/fileman/file_manager.cpp)


add_executable(${EXEC_NAME} ${SOURCE_FILES})
target_link_libraries(${EXEC_NAME} yaml-cpp)
if(UNIX)
	target_link_libraries(${EXEC_NAME} -lzmq)
	target_link_libraries(${EXEC_NAME} -lcurl)
	target_link_libraries(${EXEC_NAME} -lboost_filesystem)
	target_link_libraries(${EXEC_NAME} -lboost_system)
elseif(MSVC)
	target_link_libraries(${EXEC_NAME} ${Boost_LIBRARIES})
	target_link_libraries(${EXEC_NAME} ${ZEROMQ_LIB})
	target_link_libraries(${EXEC_NAME} ${CURL_LIBRARIES})
endif()


# Include Yaml-cpp
add_subdirectory(vendor/yaml-cpp)

# Include Google Test libraries and then our very own unit tests
add_subdirectory(vendor/googletest)
add_subdirectory(tests)
