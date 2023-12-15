# CMakeList.txt : CMake project for JojPlatform, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojGraphics)

add_library(JojGraphics graphics_dx12.cpp graphics_dx11.cpp)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojGraphics PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
	target_link_libraries(JojGraphics PRIVATE dxgi.lib d3d11.lib d3d12.lib)
endif()

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

# Link jojPlatform to JojGraphics
target_link_libraries(JojGraphics
	PRIVATE 
	JojPlatform)

if(WIN32)
	add_subdirectory(dx12)
endif()