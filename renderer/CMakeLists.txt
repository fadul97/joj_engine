# CMakeList.txt : CMake project for JojRenderer, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojRenderer)

add_library(JojRenderer renderer_dx11.cpp renderer_dx12.cpp geometry.cpp)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojRenderer PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
	target_link_libraries(JojRenderer PRIVATE dxgi.lib d3d11.lib d3d12.lib D3DCompiler.lib)
endif()

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../graphics/)

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

# Link JojGraphics and JojPlatform to JojRenderer
target_link_libraries(JojRenderer
	PRIVATE 
	JojGraphics
	JojPlatform)