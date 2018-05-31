cmake_minimum_required(VERSION 2.8)
project(recodex-token-judge)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

# Use all settings from original Makefile
if(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")
	set(CMAKE_C_LINKER_FLAGS "${CMAKE_C_LINKER_FLAGS} -s")
	use_cxx11()
endif()

# Use latest C++ in visual studio ...
if (MSVC)
	if (MSVC_VERSION GREATER_EQUAL "1900")
		include(CheckCXXCompilerFlag)
		CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
		if (_cpp_latest_flag_supported)
			add_compile_options("/std:c++latest")
		endif()
	endif()
endif()

# The worker executable
set(SOURCE_FILES
	recodex-token-judge.cpp
	reader.hpp
	comparator.hpp
	judge.hpp
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
