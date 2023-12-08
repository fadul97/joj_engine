# CMakeList.txt : CMake project for JojPlatform, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojPlatform)

add_library(JojPlatform window_win32.cpp input_win32.cpp)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojPlatform PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
	target_link_libraries(JojPlatform PRIVATE User32.lib Gdi32.lib)
endif()

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)
