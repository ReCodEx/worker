cmake_minimum_required(VERSION 2.8)
project(recodex-judge-shuffle)

# Use all settings from original Makefile
if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -fexceptions")
	set(CMAKE_CXX_LINKER_FLAGS "${CMAKE_CXX_LINKER_FLAGS} -s")
endif()

# The worker executable
set(SOURCE_FILES
	main.cpp
	token.cpp
	token.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})


# installation
if(UNIX)
	install(TARGETS recodex-judge-shuffle DESTINATION /usr/bin COMPONENT binaries)
elseif(MSVC)
	install(TARGETS recodex-judge-shuffle DESTINATION worker/bin COMPONENT binaries)
else()
	install(TARGETS recodex-judge-shuffle DESTINATION recodex/worker/bin COMPONENT binaries)
endif()
