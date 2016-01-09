project(codex_worker_tests)

set(TESTS_NAME run_tests)
set(SRC_DIR ../src)
set(LIBS_DIR ../vendor)
set(FILEMAN_DIR ../src/fileman)
set(SANDBOX_DIR ../src/sandbox)
set(TASKS_DIR ../src/tasks)

# Google Test and Google Mock headers
include_directories(${LIBS_DIR}/googletest/include)
include_directories(${LIBS_DIR}/googlemock/include)

# The main test executable
add_executable(${TESTS_NAME}
	tests_main.cpp
	broker_connection.cpp
	${SRC_DIR}/config/worker_config.cpp
	worker_config.cpp
	cache_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	http_manager.cpp
	${FILEMAN_DIR}/http_manager.cpp
	file_manager.cpp
	${FILEMAN_DIR}/file_manager.cpp
	isolate_sandbox.cpp
	${SANDBOX_DIR}/isolate_sandbox.cpp
	${SRC_DIR}/job_evaluator.cpp
	job.cpp
	topological_sort.cpp
	${TASKS_DIR}/job.cpp
	${TASKS_DIR}/task_base.cpp
	${TASKS_DIR}/external_task.cpp
	${TASKS_DIR}/fake_task.cpp
	${SRC_DIR}/job_receiver.cpp
	job_receiver.cpp
	${TASKS_DIR}/internal/cp_task.cpp
	${TASKS_DIR}/internal/mkdir_task.cpp
	${TASKS_DIR}/internal/rename_task.cpp
	${TASKS_DIR}/internal/rm_task.cpp
	${TASKS_DIR}/internal/archivate_task.cpp
	${TASKS_DIR}/internal/extract_task.cpp
	tasks.cpp
	${SRC_DIR}/archives/archivator.cpp
	archivator.cpp
)

target_link_libraries(${TESTS_NAME} gtest gtest_main)
target_link_libraries(${TESTS_NAME} gmock gmock_main)
target_link_libraries(${TESTS_NAME} yaml-cpp)
target_link_libraries(${TESTS_NAME} archive)

if(UNIX)
	target_link_libraries(${TESTS_NAME} -lzmq)
	target_link_libraries(${TESTS_NAME} -lboost_system -lboost_filesystem -lboost_program_options)
	target_link_libraries(${TESTS_NAME} -lcurl)
elseif(MSVC)
	target_link_libraries(${TESTS_NAME} ${Boost_LIBRARIES})
	target_link_libraries(${TESTS_NAME} ${ZEROMQ_LIB})
	target_link_libraries(${TESTS_NAME} ${CURL_LIBRARIES})
endif()
