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

set(BASE_LIBS
	gtest gtest_main
	gmock gmock_main
	yaml-cpp
)

if(UNIX)
	set(LIBS ${BASE_LIBS}
		-lzmq
		-lboost_system -lboost_filesystem -lboost_program_options
		archive
	)
elseif(MSVC)
	set(LIBS ${BASE_LIBS} archive_static)

	if(APPVEYOR_COMPILE)
		set(LIBS ${LIBS}
			${Boost_LIBRARIES}
			${ZEROMQ_LIB}
		)
	endif()
endif()

function(add_test_suite name)
	cmake_parse_arguments(TEST "" "WORKING_DIRECTORY" "" ${ARGN})
	add_executable(run_test_${name} ${TEST_UNPARSED_ARGUMENTS})
	target_link_libraries(run_test_${name} ${LIBS})
	if(MSVC)
		target_compile_definitions(run_test_${name} PRIVATE LIBARCHIVE_STATIC)
	endif()

	if(TEST_WORKING_DIRECTORY)
		add_test(NAME ${name} COMMAND run_test_${name} WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}")
	else()
		add_test(NAME ${name} COMMAND run_test_${name})
	endif()
endfunction()

# Unit and integration tests
add_test_suite(broker_connection
	broker_connection.cpp
	${SRC_DIR}/config/worker_config.cpp
	${HELPERS_DIR}/config.cpp
	${HELPERS_DIR}/create_logger.cpp
)

add_test_suite(worker_config
	${SRC_DIR}/config/worker_config.cpp
	worker_config.cpp
	${HELPERS_DIR}/config.cpp
)

add_test_suite(cache_manager
	cache_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	${HELPERS_DIR}/create_logger.cpp
)

add_test_suite(fallback_file_manager
	${FILEMAN_DIR}/fallback_file_manager.cpp
	fallback_file_manager.cpp
	${HELPERS_DIR}/create_logger.cpp
)

add_test_suite(job
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/task_base.cpp
	${SRC_DIR}/archives/archivator.cpp
	${SRC_DIR}/config/worker_config.cpp
	${SANDBOX_DIR}/fake_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/create_logger.cpp
	${HELPERS_DIR}/config.cpp
	${JOB_DIR}/job.cpp
	job.cpp
)

add_test_suite(build_job_metadata
	${HELPERS_DIR}/config.cpp
	build_job_metadata.cpp
)

add_test_suite(topological_sort
	${TASKS_DIR}/task_base.cpp
	${HELPERS_DIR}/topological_sort.cpp
	topological_sort.cpp
)

add_test_suite(job_receiver
	${JOB_DIR}/job.cpp
	${JOB_DIR}/job_evaluator.cpp
	${JOB_DIR}/job_receiver.cpp
	${CONFIG_DIR}/worker_config.cpp
	${FILEMAN_DIR}/prefixed_file_manager.cpp
	${FILEMAN_DIR}/fallback_file_manager.cpp
	${HELPERS_DIR}/config.cpp
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/zmq_socket.cpp
	${HELPERS_DIR}/create_logger.cpp
	${JOB_DIR}/job.cpp
	${SRC_DIR}/archives/archivator.cpp
	${TASKS_DIR}/task_factory.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/root_task.cpp
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.cpp
	${TASKS_DIR}/internal/fetch_task.cpp
	${SANDBOX_DIR}/fake_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	job_receiver.cpp
)

add_test_suite(tasks
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.cpp
	${TASKS_DIR}/internal/fetch_task.cpp
	${SRC_DIR}/archives/archivator.cpp
	${SANDBOX_DIR}/fake_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${HELPERS_DIR}/create_logger.cpp
	tasks.cpp
)

add_test_suite(job_config
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/config.cpp
	${TASKS_DIR}/task_base.cpp
	job_config.cpp
)

# Tests that depend on external resources
if(UNIX)
	set(LIBS
		gtest gmock
		yaml-cpp
		-lcurl
		-lzmq
		-lboost_system -lboost_filesystem -lboost_program_options
		archive
		)
elseif(MSVC)
	set(LIBS gtest gmock
		yaml-cpp
		archive_static
	)

	if(APPVEYOR_COMPILE)
		set(LIBS ${LIBS}
			${CURL_LIBRARIES}
			${Boost_LIBRARIES}
			${ZEROMQ_LIB}
		)
	endif()
endif()

add_test_suite(tool_isolate_sandbox
	tests_main.cpp
	isolate_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${HELPERS_DIR}/create_logger.cpp
)

add_test_suite(tool_archivator
	tests_main.cpp
	${SRC_DIR}/archives/archivator.cpp
	archivator.cpp
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
)

add_test_suite(tool_http_manager
	tests_main.cpp
	http_manager.cpp
	${FILEMAN_DIR}/http_manager.cpp
	${HELPERS_DIR}/create_logger.cpp
)
