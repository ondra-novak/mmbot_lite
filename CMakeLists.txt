cmake_minimum_required(VERSION 3.1)
project (mmbot_lite)

if (MSVC) 
	add_compile_options(/W4 /EHsc /DNOMINMAX /std:c++20)
	set(STANDARD_LIBRARIES "")
else()
	add_compile_options(-Wall -Wextra -Wpedantic -std=c++20)
	set(STANDARD_LIBRARIES "pthread")
endif()

include_directories(BEFORE src/json20/src src/docdb/src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/ CACHE STRING "Output directory")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/ CACHE STRING "Library directory")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/ CACHE STRING "Archive directory")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Default path to install" FORCE)
endif()
add_subdirectory("src/mmbot_lite")
