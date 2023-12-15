# CMakeList.txt : CMake project for JojPlatform, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojPlatform)

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

if(WIN32)
	add_subdirectory(win32)
endif()