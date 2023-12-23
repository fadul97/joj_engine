# CMakeList.txt : CMake project for JojWin32Platform, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojWin32Platform)

add_library(JojWin32Platform window.cpp input.cpp timer.cpp)

target_link_libraries(JojWin32Platform PRIVATE User32.lib Gdi32.lib winmm.lib)

target_include_directories(JojWin32Platform INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojWin32Platform PROPERTY CXX_STANDARD 20)
endif()