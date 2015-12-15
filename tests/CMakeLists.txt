project(codex_worker_tests)

set(SRC_DIR ../src)
set(LIBS_DIR ../vendor)
set(FILEMAN_DIR ../src/fileman)

# Google Test and Google Mock headers
include_directories(${LIBS_DIR}/googletest/include)
include_directories(${LIBS_DIR}/googlemock/include)

# The main test executable
add_executable(run_tests
	broker_connection.cpp
	${SRC_DIR}/worker_config.cpp
	worker_config.cpp
	cache_manager.cpp
	${FILEMAN_DIR}/cache_manager.cpp
	http_manager.cpp
	${FILEMAN_DIR}/http_manager.cpp
	file_manager.cpp
	${FILEMAN_DIR}/file_manager.cpp)

target_link_libraries(run_tests gtest gtest_main)
target_link_libraries(run_tests gmock gmock_main)
target_link_libraries(run_tests yaml-cpp)
target_link_libraries(run_tests -lzmq)
target_link_libraries(run_tests -lboost_system)
target_link_libraries(run_tests -lboost_filesystem)
target_link_libraries(run_tests -lcurl)
