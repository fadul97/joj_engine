# CMakeList.txt : CMake project for JojRenderer, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojRenderer)

add_library(JojRenderer geometry.cpp)

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../graphics/)

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

# Link JojGraphics and JojPlatform to JojRenderer
target_link_libraries(JojRenderer PRIVATE JojGraphics JojPlatform)

if(WIN32)
	# Include dx11 folder
	add_subdirectory(dx11)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dx11/)

	# Include dx12 folder
	add_subdirectory(dx12)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dx12/)

	target_link_libraries(JojGraphics INTERFACE JojRendererD3D11 JojRendererD3D12)
endif()