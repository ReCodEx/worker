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
	set(TEST_FILES ${TEST_FILES} ${ARGN} PARENT_SCOPE)
	add_executable(run_test_${name} ${ARGN})
	target_link_libraries(run_test_${name} ${LIBS})
	if (MSVC)
		target_compile_definitions(run_test_${name} PRIVATE LIBARCHIVE_STATIC)
	endif()
endfunction()

# Unit and integration tests
add_test_suite(broker_connection
	broker_connection.cpp
	${SRC_DIR}/config/worker_config.cpp
)

add_test_suite(worker_config
	${SRC_DIR}/config/worker_config.cpp
	worker_config.cpp
)

add_test_suite(cache_manager
	cache_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
)

add_test_suite(fallback_file_manager
	${FILEMAN_DIR}/fallback_file_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	fallback_file_manager.cpp
)

add_test_suite(job
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.cpp
	${TASKS_DIR}/internal/fetch_task.cpp
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/fake_task.cpp
	${SRC_DIR}/archives/archivator.cpp
	${SRC_DIR}/config/worker_config.cpp
	${HELPERS_DIR}/config.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	${SANDBOX_DIR}/fake_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${HELPERS_DIR}/topological_sort.cpp
	${JOB_DIR}/job.cpp
	job.cpp
)

add_test_suite(topological_sort
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/fake_task.cpp
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
	${JOB_DIR}/job.cpp
	${SRC_DIR}/archives/archivator.cpp
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/fake_task.cpp
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
	${TASKS_DIR}/fake_task.cpp
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
	tasks.cpp
)

add_test_suite(job_config
	${HELPERS_DIR}/topological_sort.cpp
	${HELPERS_DIR}/filesystem.cpp
	${HELPERS_DIR}/config.cpp
	${TASKS_DIR}/task_base.cpp
	job_config.cpp
)

list(REMOVE_DUPLICATES TEST_FILES)
add_executable(${TESTS_NAME} ${TEST_FILES})
target_link_libraries(${TESTS_NAME} ${LIBS})
target_compile_definitions(${TESTS_NAME} PRIVATE LIBARCHIVE_STATIC)

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

unset(TEST_FILES)

add_test_suite(isolate_sandbox
	tests_main.cpp
	isolate_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
)

add_test_suite(archivator
	tests_main.cpp
	${SRC_DIR}/archives/archivator.cpp
	archivator.cpp
)

add_test_suite(http_manager
	tests_main.cpp
	http_manager.cpp
	${FILEMAN_DIR}/http_manager.cpp
)

list(REMOVE_DUPLICATES TEST_FILES)
add_executable(${TOOL_TESTS_NAME}
	${TEST_FILES}
)

target_link_libraries(${TOOL_TESTS_NAME} ${LIBS})
target_compile_definitions(${TOOL_TESTS_NAME} PRIVATE LIBARCHIVE_STATIC)
