cmake_minimum_required(VERSION 2.8.11)
project(codex_worker)


set(EXEC_NAME ${PROJECT_NAME})
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable yaml-cpp testing and parse tools")
set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib.")

# Find installed packages on system
# -- LibArchive
find_package(LibArchive)

# -- Boost
if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
else()
	set(Boost_USE_STATIC_LIBS OFF)
endif()
set (Boost_USE_MULTITHREADED ON)
find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system program_options)
include_directories(${Boost_INCLUDE_DIRS})

# -- cURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# -- Yaml-cpp
# find the yaml-cpp include directory
find_path(YAMLCPP_INCLUDE_DIR yaml-cpp/yaml.h
		  PATH_SUFFIXES include
		  PATHS
		  ~/Library/Frameworks/yaml-cpp/include/
		  /Library/Frameworks/yaml-cpp/include/
		  /usr/local/include/
		  /usr/include/
		  /sw/yaml-cpp/         # Fink
		  /opt/local/yaml-cpp/  # DarwinPorts
		  /opt/csw/yaml-cpp/    # Blastwave
		  /opt/yaml-cpp/
		  ${YAMLCPP_DIR}/include/)

# find the yaml-cpp library
find_library(YAMLCPP_LIBRARY
			 NAMES ${YAMLCPP_STATIC} yaml-cpp
			 PATH_SUFFIXES lib64 lib
			 PATHS ~/Library/Frameworks
					/Library/Frameworks
					/usr/local
					/usr
					/sw
					/opt/local
					/opt/csw
					/opt
					${YAMLCPP_DIR}/lib)

if(YAMLCPP_INCLUDE_DIR AND YAMLCPP_LIBRARY)
	set(YAMLCPP_FOUND true)
endif()


# MS Visual C++ specific bindings
if(MSVC)	
	# ZeroMQ on Windows...
	find_path(ZEROMQ_INCLUDE NAMES zmq.h PATHS "C:\\deps\\libzmq\\include")
	find_library(ZEROMQ_LIB NAMES libzmq PATHS "C:\\deps\\libzmq\\lib")
	
	if(ZEROMQ_LIB)
		include_directories(${ZEROMQ_INCLUDE})
	else()
		message(FATAL_ERROR "Fail to load ZeroMQ!")
	endif()
endif()


# ZeroMQ C++ bindings headers
include_directories(AFTER, vendor/cppzmq)

# Spdlog logging library
include_directories(AFTER, vendor/spdlog/include)

# Yaml-cpp
if(YAMLCPP_FOUND)
	include_directories(AFTER, ${YAMLCPP_INCLUDE_DIR})
else()
	include_directories(AFTER, vendor/yaml-cpp/include)
endif()

# Libarchive
if(LibArchive_FOUND)
	include_directories(AFTER, ${LibArchive_INCLUDE_DIRS})
else()
	include_directories(AFTER, vendor/libarchive/libarchive)
endif()

# Use C++11
if(UNIX)
    #-Wno-deprecated-declarations hides warning in yaml-cpp (using std::auto_ptr)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-deprecated-declarations")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# The worker executable
set(SOURCE_FILES
	src/main.cpp
	src/config/worker_config.cpp
	src/config/worker_config.h
	src/config/fileman_config.h
	src/config/log_config.h
	src/config/sandbox_limits.h
	src/config/task_results.h
	src/broker_connection.h
	src/connection_proxy.h
	src/fileman/file_manager_base.h
	src/fileman/cache_manager.h
	src/fileman/http_manager.h
	src/fileman/file_manager.h
	src/fileman/cache_manager.cpp
	src/fileman/http_manager.cpp
	src/fileman/file_manager.cpp
    src/fileman/prefixed_file_manager.cpp
    src/fileman/prefixed_file_manager.h
	src/sandbox/sandbox_base.h
	src/isoeval_core.h
	src/isoeval_core.cpp
	src/tasks/job.h
	src/tasks/job.cpp
	src/tasks/fake_task.h
	src/tasks/fake_task.cpp
	src/tasks/task_base.h
	src/tasks/task_base.cpp
	src/tasks/external_task.h
	src/tasks/external_task.cpp
	src/tasks/internal/cp_task.h
	src/tasks/internal/cp_task.cpp
	src/tasks/internal/rename_task.h
	src/tasks/internal/rename_task.cpp
	src/tasks/internal/mkdir_task.h
	src/tasks/internal/mkdir_task.cpp
	src/tasks/internal/rm_task.h
	src/tasks/internal/rm_task.cpp
	src/tasks/internal/archivate_task.h
	src/tasks/internal/archivate_task.cpp
	src/tasks/internal/extract_task.h
	src/tasks/internal/extract_task.cpp
	src/tasks/internal/fetch_task.h
	src/tasks/internal/fetch_task.cpp
	src/sandbox/isolate_sandbox.h
	src/sandbox/isolate_sandbox.cpp
	src/job_evaluator.h
	src/job_evaluator.cpp
	src/job_receiver.cpp
	src/job_receiver.h
	src/eval_request.h
	src/archives/archivator.h
	src/archives/archivator.cpp
	src/eval_response.h
	src/helpers/filesystem.h
	src/helpers/filesystem.cpp
	src/helpers/topological_sort.h
	src/helpers/topological_sort.cpp
)


add_executable(${EXEC_NAME} ${SOURCE_FILES})

if(YAMLCPP_FOUND)
	target_link_libraries(${EXEC_NAME} ${YAMLCPP_LIBRARY})
else()
	target_link_libraries(${EXEC_NAME} yaml-cpp)
endif()

if(LibArchive_FOUND)
	target_link_libraries(${EXEC_NAME} ${LibArchive_LIBRARIES})
else()
	target_link_libraries(${EXEC_NAME} archive)
endif()

target_link_libraries(${EXEC_NAME} ${CURL_LIBRARIES})
target_link_libraries(${EXEC_NAME} ${Boost_LIBRARIES})

if(UNIX)
	target_link_libraries(${EXEC_NAME} -lzmq)
	target_link_libraries(${EXEC_NAME} pthread)
elseif(MSVC)
	target_link_libraries(${EXEC_NAME} ${ZEROMQ_LIB})	
endif()


if(NOT LibArchive_FOUND)
	# Include libarchive
	option(ENABLE_CPIO OFF)
	option(ENABLE_TAR OFF)
	option(ENABLE_TEST OFF)
	add_subdirectory(vendor/libarchive)
endif()

if(NOT YAMLCPP_FOUND)
	# Include Yaml-cpp
	add_subdirectory(vendor/yaml-cpp)
endif()

# Include Google Test libraries and then our very own unit tests
add_subdirectory(vendor/googletest)
add_subdirectory(tests)

# Include judges
add_subdirectory(judges)
