project(codex_worker_tests)

set(TESTS_NAME run_tests)
set(SRC_DIR ../src)
set(LIBS_DIR ../vendor)
set(FILEMAN_DIR ../src/fileman)

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
	${FILEMAN_DIR}/file_manager.cpp)

target_link_libraries(${TESTS_NAME} gtest gtest_main)
target_link_libraries(${TESTS_NAME} gmock gmock_main)
target_link_libraries(${TESTS_NAME} yaml-cpp)

if(UNIX)
	target_link_libraries(${TESTS_NAME} -lzmq)
	target_link_libraries(${TESTS_NAME} -lboost_system)
	target_link_libraries(${TESTS_NAME} -lboost_filesystem)
	target_link_libraries(${TESTS_NAME} -lcurl)
elseif(MSVC)
	target_link_libraries(${TESTS_NAME} ${Boost_LIBRARIES})
	target_link_libraries(${TESTS_NAME} ${ZEROMQ_LIB})
	target_link_libraries(${TESTS_NAME} ${CURL_LIBRARIES})
endif()
