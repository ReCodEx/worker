cmake_minimum_required(VERSION 3.11.0)
project(recodex-worker)
set(RECODEX_VERSION 1.8.0)
enable_testing()

set(EXEC_NAME ${PROJECT_NAME})
set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib.")

# Find installed packages on the system
# -- LibArchive
find_package(LibArchive)

# -- Boost
if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
else()
	set(Boost_USE_STATIC_LIBS OFF)
endif()
# -- load Boost
set (Boost_USE_MULTITHREADED ON)
find_package(Boost 1.74.0 REQUIRED COMPONENTS system program_options)
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


# -- ZeroMQ on Windows
if(MSVC)
	find_path(ZEROMQ_INCLUDE NAMES zmq.h PATHS ${ZMQ_INCLUDE_DIR})
	find_library(ZEROMQ_LIB NAMES libzmq PATHS ${ZMQ_LIBRARY_DIR})

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

# Use C++17
if(UNIX)
	#-Wno-deprecated-declarations hides warning in yaml-cpp (using std::auto_ptr)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wno-deprecated-declarations")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /EHsc /MP /D NOMINMAX")
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
set(COMMAND_DIR src/commands)

# The worker executable
set(SOURCE_FILES
	${SRC_DIR}/main.cpp
	${SRC_DIR}/broker_connection.h
	${SRC_DIR}/connection_proxy.h
	${SRC_DIR}/worker_core.h
	${SRC_DIR}/worker_core.cpp
	${SRC_DIR}/eval_request.h
	${SRC_DIR}/archives/archivator.h
	${SRC_DIR}/archives/archivator.cpp
	${SRC_DIR}/eval_response.h

	${FILEMAN_DIR}/file_manager_interface.h
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

	${TASKS_DIR}/task_factory_interface.h
	${TASKS_DIR}/create_params.h
	${TASKS_DIR}/task_factory.h
	${TASKS_DIR}/task_factory.cpp
	${TASKS_DIR}/root_task.h
	${TASKS_DIR}/root_task.cpp
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
	${TASKS_DIR}/internal/dump_dir_task.h
	${TASKS_DIR}/internal/dump_dir_task.cpp
	${TASKS_DIR}/internal/truncate_task.h
	${TASKS_DIR}/internal/truncate_task.cpp
	${TASKS_DIR}/internal/exists_task.h
	${TASKS_DIR}/internal/exists_task.cpp

	${HELPERS_DIR}/filesystem.h
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/topological_sort.h
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/config.h
	${HELPERS_DIR}/config.cpp
	${HELPERS_DIR}/zmq_socket.h
	${HELPERS_DIR}/zmq_socket.cpp
	${HELPERS_DIR}/logger.h
	${HELPERS_DIR}/logger.cpp
	${HELPERS_DIR}/string_utils.h
	${HELPERS_DIR}/string_utils.cpp
	${HELPERS_DIR}/type_utils.h
	${HELPERS_DIR}/format.h

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
	${JOB_DIR}/job_evaluator_interface.h
	${JOB_DIR}/job_evaluator.h
	${JOB_DIR}/job_evaluator.cpp
	${JOB_DIR}/job_receiver.cpp
	${JOB_DIR}/job_receiver.h
	${JOB_DIR}/progress_callback_interface.h
	${JOB_DIR}/progress_callback.h
	${JOB_DIR}/progress_callback.cpp

	${COMMAND_DIR}/command_holder.h
	${COMMAND_DIR}/broker_commands.h
	${COMMAND_DIR}/jobs_server_commands.h
	${COMMAND_DIR}/jobs_client_commands.h
)

include_directories(AFTER, ${SRC_DIR})


add_executable(${EXEC_NAME} ${SOURCE_FILES})

if(YAMLCPP_FOUND)
	target_link_libraries(${EXEC_NAME} ${YAMLCPP_LIBRARY})
else()
	target_link_libraries(${EXEC_NAME} yaml-cpp)
endif()

if(LibArchive_FOUND)
	target_link_libraries(${EXEC_NAME} ${LibArchive_LIBRARIES})
elseif(UNIX)
	target_link_libraries(${EXEC_NAME} archive)
else()
	target_link_libraries(${EXEC_NAME} archive_static)
	target_compile_definitions(${EXEC_NAME} PRIVATE LIBARCHIVE_STATIC)
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
	option(YAML_CPP_BUILD_TOOLS OFF)
	add_subdirectory(vendor/yaml-cpp)
endif()

if(NOT DISABLE_TESTS)
	# Include Google Test libraries and then our very own unit tests
	add_subdirectory(vendor/googletest EXCLUDE_FROM_ALL)
	add_subdirectory(tests)
endif()

# Include judges
add_subdirectory(judges)


# ========== Install targets - 'sudo make install' ==========
include(InstallRequiredSystemLibraries)
if(UNIX)
	install(TARGETS recodex-worker DESTINATION /usr/bin COMPONENT binaries)
	install(FILES install/recodex-worker@.service DESTINATION /lib/systemd/system COMPONENT config)
	install(FILES examples/config.yml DESTINATION /etc/recodex/worker COMPONENT config RENAME config-1.yml)
elseif(MSVC)
	install(TARGETS recodex-worker DESTINATION worker/bin COMPONENT binaries)
	install(DIRECTORY build/Release/ DESTINATION worker/bin COMPONENT binaries FILES_MATCHING PATTERN "*.dll")
	install(FILES build/curl-ca-bundle.crt DESTINATION worker/bin COMPONENT binaries)
	install(FILES examples/config.yml DESTINATION worker/conf COMPONENT config RENAME config-1.yml)
else()
	install(TARGETS recodex-worker DESTINATION recodex/worker/bin COMPONENT binaries)
	install(FILES examples/config.yml DESTINATION recodex/worker/conf COMPONENT config RENAME config-1.yml)
endif()


# ========== Packaging targets - 'sudo make package' ==========
set(CPACK_PACKAGE_NAME "recodex-worker")
set(CPACK_PACKAGE_VENDOR "ReCodEx Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ReCodEx Worker - Backend part of ReCodEx programmer testing solution")
set(CPACK_COMPONENTS_ALL "binaries" "config")
set(CPACK_PACKAGE_VERSION ${RECODEX_VERSION})
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_PACKAGE_RELOCATABLE "FALSE")
if(UNIX)
	set(CPACK_GENERATOR "RPM" "DEB")
	set(CPACK_RPM_PACKAGE_LICENSE "MIT")
	set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
	set(CPACK_RPM_PACKAGE_RELEASE "1")
	set(CPACK_RPM_PACKAGE_DESCRIPTION "Backend part of ReCodEx programmer testing solution.")
	set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install/postinst")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Petr Stefan <ps@stdin.cz>")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
	set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/install/postinst;")
	set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${RECODEX_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}-${CPACK_RPM_PACKAGE_ARCHITECTURE}")
elseif(MSVC)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
endif()
include (CPack)

# ========== Formatting ==========
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.h tests/*.cpp tests/*.h)
if(UNIX)
	add_custom_target(format
		COMMAND clang-format --style=file -i ${ALL_SOURCE_FILES}
		COMMENT "Running clang-format"
		VERBATIM
	)
endif()

# Count sum of lines in source files (for Martin)
add_custom_target(lines
	COMMAND echo -n "Total lines of code: " && cat ${ALL_SOURCE_FILES} | wc -l
	COMMENT "Counting lines"
	VERBATIM
)

