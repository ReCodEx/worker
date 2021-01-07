cmake_minimum_required(VERSION 3.11.0)
project(recodex-judge-passthrough)

# Use all settings from original Makefile
if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -fexceptions")
	set(CMAKE_CXX_LINKER_FLAGS "${CMAKE_CXX_LINKER_FLAGS} -s")
endif()

# The worker executable
set(SOURCE_FILES
	main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})


# installation
if(UNIX)
	install(TARGETS recodex-judge-passthrough DESTINATION /usr/bin COMPONENT binaries)
	install(PROGRAMS recodex-data-only-wrapper.sh DESTINATION /usr/bin COMPONENT binaries)
elseif(MSVC)
	install(TARGETS recodex-judge-passthrough DESTINATION worker/bin COMPONENT binaries)
	install(PROGRAMS recodex-data-only-wrapper.sh DESTINATION worker/bin COMPONENT binaries)
else()
	install(TARGETS recodex-judge-passthrough DESTINATION recodex/worker/bin COMPONENT binaries)
	install(PROGRAMS recodex-data-only-wrapper.sh DESTINATION recodex/worker/bin COMPONENT binaries)
endif()

