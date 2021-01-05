cmake_minimum_required(VERSION 3.19.0)
project(recodex-judge-filter)

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

# installation
if(UNIX)
	install(TARGETS recodex-judge-filter DESTINATION /usr/bin COMPONENT binaries)
elseif(MSVC)
	install(TARGETS recodex-judge-filter DESTINATION worker/bin COMPONENT binaries)
else()
	install(TARGETS recodex-judge-filter DESTINATION recodex/worker/bin COMPONENT binaries)
endif()
