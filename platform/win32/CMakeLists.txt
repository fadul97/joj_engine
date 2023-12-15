# CMakeList.txt : CMake project for JojPlatform, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojPlatform)

add_library(JojPlatform window.cpp input.cpp timer.cpp)

target_link_libraries(JojPlatform PRIVATE User32.lib Gdi32.lib winmm.lib)

target_include_directories(JojPlatform INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojPlatform PROPERTY CXX_STANDARD 20)
endif()