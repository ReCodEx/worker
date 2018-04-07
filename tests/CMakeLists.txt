project(recodex-worker_tests)

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
		-lgcov --coverage
		archive
	)
elseif(MSVC)
	set(LIBS ${BASE_LIBS} archive_static ${Boost_LIBRARIES} ${ZEROMQ_LIB})
endif()

function(add_test_suite name)
	cmake_parse_arguments(TEST "" "WORKING_DIRECTORY" "" ${ARGN})
	add_executable(run_test_${name} ${TEST_UNPARSED_ARGUMENTS})
	if(UNIX)
		set_target_properties(run_test_${name} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage --coverage")
	endif()
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
	mocks.h
	broker_connection.cpp
	${SRC_DIR}/config/worker_config.cpp
	${HELPERS_DIR}/config.cpp
	${HELPERS_DIR}/logger.cpp
)

add_test_suite(worker_config
	${SRC_DIR}/config/worker_config.cpp
	worker_config.cpp
	${HELPERS_DIR}/config.cpp
)

add_test_suite(cache_manager
	cache_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	${HELPERS_DIR}/logger.cpp
)

add_test_suite(fallback_file_manager
	mocks.h
	${FILEMAN_DIR}/fallback_file_manager.cpp
	fallback_file_manager.cpp
	${HELPERS_DIR}/logger.cpp
)

add_test_suite(job
	mocks.h
	${TASKS_DIR}/task_base.cpp
	${SRC_DIR}/archives/archivator.cpp
	${SRC_DIR}/config/worker_config.cpp
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/logger.cpp
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
	mocks.h
	${JOB_DIR}/job_receiver.cpp
	${HELPERS_DIR}/zmq_socket.cpp
	${HELPERS_DIR}/logger.cpp
	job_receiver.cpp
)

add_test_suite(tasks
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/task_factory.cpp
	${TASKS_DIR}/root_task.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/dump_dir_task.cpp
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.cpp
	${TASKS_DIR}/internal/fetch_task.cpp
	${TASKS_DIR}/internal/truncate_task.cpp
	${TASKS_DIR}/internal/exists_task.cpp
	${SRC_DIR}/archives/archivator.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${HELPERS_DIR}/logger.cpp
	${HELPERS_DIR}/config.cpp
	${HELPERS_DIR}/string_utils.cpp
	${HELPERS_DIR}/filesystem.cpp
	${CONFIG_DIR}/worker_config.cpp
	tasks.cpp
)

add_test_suite(job_config
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/config.cpp
	${TASKS_DIR}/task_base.cpp
	job_config.cpp
)

add_test_suite(progress_callback
	${JOB_DIR}/progress_callback.cpp
	${HELPERS_DIR}/zmq_socket.cpp
	${HELPERS_DIR}/logger.cpp
	progress_callback.cpp
)

add_test_suite(filesystem
	${HELPERS_DIR}/filesystem.cpp
	filesystem.cpp
)

add_test_suite(string_utils
	${HELPERS_DIR}/string_utils.cpp
	string_utils.cpp
)

add_test_suite(dump_dir_task
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/internal/dump_dir_task.cpp
	dump_dir_task.cpp
)

add_test_suite(truncate_task
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/internal/truncate_task.cpp
	truncate_task.cpp
)

add_test_suite(exists_task
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/internal/exists_task.cpp
	exists_task.cpp
)

# Tests that depend on external resources
if(UNIX)
	set(LIBS
		gtest gmock
		yaml-cpp
		-lcurl
		-lzmq
		-lboost_system -lboost_filesystem -lboost_program_options
		-lgcov --coverage
		archive
	)
elseif(MSVC)
	set(LIBS gtest gmock
		yaml-cpp
		archive_static
		${CURL_LIBRARIES}
		${Boost_LIBRARIES}
		${ZEROMQ_LIB}
	)
endif()

add_test_suite(tool_isolate_sandbox
	tests_main.cpp
	isolate_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${HELPERS_DIR}/logger.cpp
	${HELPERS_DIR}/filesystem.cpp
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
	${HELPERS_DIR}/logger.cpp
)
