# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)
project(JojPlatform)

add_library(JojPlatform window_win32.cpp)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojPlatform PROPERTY CXX_STANDARD 20)
endif()
