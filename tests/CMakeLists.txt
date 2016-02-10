project(codex_worker_tests)

set(TESTS_NAME run_tests)
set(TOOL_TESTS_NAME run_tool_tests)
set(SRC_DIR ../src)
set(LIBS_DIR ../vendor)
set(FILEMAN_DIR ../src/fileman)
set(SANDBOX_DIR ../src/sandbox)
set(TASKS_DIR ../src/tasks)
set(HELPERS_DIR ../src/helpers)
set(CONFIG_DIR ../src/config)
set(JOB_DIR ../src/job)

# Google Test and Google Mock headers
include_directories(${LIBS_DIR}/googletest/include)
include_directories(${LIBS_DIR}/googlemock/include)

# Unit and integration tests
add_executable(${TESTS_NAME}
	broker_connection.cpp
	${SRC_DIR}/config/worker_config.cpp
	worker_config.cpp
	cache_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	${FILEMAN_DIR}/prefixed_file_manager.cpp
	${FILEMAN_DIR}/fallback_file_manager.cpp
	fallback_file_manager.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${SANDBOX_DIR}/fake_sandbox.cpp
	${JOB_DIR}/job_evaluator.cpp
	job.cpp
	topological_sort.cpp
	${JOB_DIR}/job.cpp
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/fake_task.cpp
	${JOB_DIR}/job_receiver.cpp
	job_receiver.cpp
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.cpp
	${TASKS_DIR}/internal/fetch_task.cpp
	tasks.cpp
	${SRC_DIR}/archives/archivator.cpp
	${HELPERS_DIR}/topological_sort.h
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/filesystem.h
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/config.h
	${HELPERS_DIR}/config.cpp
	job_config.cpp
)

target_link_libraries(${TESTS_NAME} gtest gtest_main)
target_link_libraries(${TESTS_NAME} gmock gmock_main)
target_link_libraries(${TESTS_NAME} yaml-cpp)

if(UNIX)
	target_link_libraries(${TESTS_NAME} -lzmq)
	target_link_libraries(${TESTS_NAME} -lboost_system -lboost_filesystem -lboost_program_options)
	target_link_libraries(${TESTS_NAME} archive)
elseif(MSVC)
	target_link_libraries(${TESTS_NAME} archive_static)
	target_compile_definitions(${TESTS_NAME} PRIVATE LIBARCHIVE_STATIC)
	if(APPVEYOR_COMPILE)
		target_link_libraries(${TESTS_NAME} ${Boost_LIBRARIES})
		target_link_libraries(${TESTS_NAME} ${ZEROMQ_LIB})
		target_link_libraries(${TESTS_NAME} ${CURL_LIBRARIES})
	endif()
endif()

# Tests that depend on external resources
add_executable(${TOOL_TESTS_NAME}
	tests_main.cpp
	isolate_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${SRC_DIR}/archives/archivator.cpp
	archivator.cpp
	http_manager.cpp
	${FILEMAN_DIR}/http_manager.cpp
)

target_link_libraries(${TOOL_TESTS_NAME} gtest)
target_link_libraries(${TOOL_TESTS_NAME} gmock)

if(UNIX)
	target_link_libraries(${TOOL_TESTS_NAME} -lzmq)
	target_link_libraries(${TOOL_TESTS_NAME} -lboost_system -lboost_filesystem -lboost_program_options)

	target_link_libraries(${TOOL_TESTS_NAME} -lcurl)
	target_link_libraries(${TOOL_TESTS_NAME} archive)
elseif(MSVC)
	target_link_libraries(${TOOL_TESTS_NAME} archive_static)
	target_compile_definitions(${TOOL_TESTS_NAME} PRIVATE LIBARCHIVE_STATIC)
	if(APPVEYOR_COMPILE)
		target_link_libraries(${TOOL_TESTS_NAME} ${Boost_LIBRARIES})
		target_link_libraries(${TOOL_TESTS_NAME} ${ZEROMQ_LIB})
		target_link_libraries(${TOOL_TESTS_NAME} ${CURL_LIBRARIES})
	endif()
endif()
