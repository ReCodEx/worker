cmake_minimum_required(VERSION 2.8)
project(codex_filter)

# Use all settings from original Makefile
if(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")
	set(CMAKE_C_LINKER_FLAGS "${CMAKE_C_LINKER_FLAGS} -s")
endif()


set(SOURCE_FILES
	io.h
	io.c
	main.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
