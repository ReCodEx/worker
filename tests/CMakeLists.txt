project(codex_worker_tests)

set(SRC_DIR ../src)
set(LIBS_DIR ../vendor)

# Google Test and Google Mock headers
include_directories(${LIBS_DIR}/googletest/include)
include_directories(${LIBS_DIR}/googlemock/include)

# The main test executable
add_executable(run_tests
        broker_connection.cpp
        ${SRC_DIR}/worker_config.cpp
)

target_link_libraries(run_tests gtest gtest_main)
target_link_libraries(run_tests gmock gmock_main)
