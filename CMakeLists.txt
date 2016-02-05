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


# Bit of sets of handy folders
set(SRC_DIR src)
set(LIBS_DIR vendor)
set(FILEMAN_DIR src/fileman)
set(SANDBOX_DIR src/sandbox)
set(TASKS_DIR src/tasks)
set(HELPERS_DIR src/helpers)
set(CONFIG_DIR src/config)
set(JOB_DIR src/job)

# The worker executable
set(SOURCE_FILES
	${SRC_DIR}/main.cpp
	${SRC_DIR}/broker_connection.h
	${SRC_DIR}/connection_proxy.h
	${SRC_DIR}/isoeval_core.h
	${SRC_DIR}/isoeval_core.cpp
	${SRC_DIR}/eval_request.h
	${SRC_DIR}/archives/archivator.h
	${SRC_DIR}/archives/archivator.cpp
	${SRC_DIR}/eval_response.h

	${FILEMAN_DIR}/file_manager_base.h
	${FILEMAN_DIR}/cache_manager.h
	${FILEMAN_DIR}/http_manager.h
	${FILEMAN_DIR}/fallback_file_manager.h
	${FILEMAN_DIR}/cache_manager.cpp
	${FILEMAN_DIR}/http_manager.cpp
	${FILEMAN_DIR}/fallback_file_manager.cpp
	${FILEMAN_DIR}/prefixed_file_manager.cpp
	${FILEMAN_DIR}/prefixed_file_manager.h

	${SANDBOX_DIR}/sandbox_base.h
	${SANDBOX_DIR}/isolate_sandbox.h
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${SANDBOX_DIR}/fake_sandbox.h
	${SANDBOX_DIR}/fake_sandbox.cpp

	${TASKS_DIR}/fake_task.h
	${TASKS_DIR}/fake_task.cpp
	${TASKS_DIR}/task_base.h
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.h
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/internal/cp_task.h
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/rename_task.h
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/mkdir_task.h
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rm_task.h
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.h
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.h
	${TASKS_DIR}/internal/extract_task.cpp
	${TASKS_DIR}/internal/fetch_task.h
	${TASKS_DIR}/internal/fetch_task.cpp

	${HELPERS_DIR}/filesystem.h
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/topological_sort.h
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/config.h
	${HELPERS_DIR}/config.cpp

	${CONFIG_DIR}/worker_config.cpp
	${CONFIG_DIR}/worker_config.h
	${CONFIG_DIR}/fileman_config.h
	${CONFIG_DIR}/log_config.h
	${CONFIG_DIR}/sandbox_limits.h
	${CONFIG_DIR}/task_results.h
	${CONFIG_DIR}/job_metadata.h
	${CONFIG_DIR}/task_metadata.h
	${CONFIG_DIR}/sandbox_config.h

	${JOB_DIR}/job.h
	${JOB_DIR}/job_exception.h
	${JOB_DIR}/job.cpp
	${JOB_DIR}/job_evaluator.h
	${JOB_DIR}/job_evaluator.cpp
	${JOB_DIR}/job_receiver.cpp
	${JOB_DIR}/job_receiver.h
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
