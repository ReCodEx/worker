cmake_minimum_required(VERSION 2.8)
project(recodex-token-judge)

# Use all settings from original Makefile
if(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")
	set(CMAKE_C_LINKER_FLAGS "${CMAKE_C_LINKER_FLAGS} -s")
endif()

# The worker executable
set(SOURCE_FILES
	recodex-token-judge.cpp
	reader.hpp
	comparator.hpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(AFTER bpplib)

# installation
if(UNIX)
	install(TARGETS recodex-token-judge DESTINATION /usr/bin COMPONENT binaries)
elseif(MSVC)
	install(TARGETS recodex-token-judge DESTINATION worker/bin COMPONENT binaries)
else()
	install(TARGETS recodex-token-judge DESTINATION recodex/worker/bin COMPONENT binaries)
endif()
