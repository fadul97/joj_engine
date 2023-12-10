# CMakeList.txt : CMake project for JojRenderer, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojRenderer)

add_library(JojRenderer renderer_dx11.cpp)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojRenderer PROPERTY CXX_STANDARD 20)
endif()

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../graphics/)

# Link JojGraphics to JojRenderer
target_link_libraries(JojRenderer
	PRIVATE 
	JojGraphics)