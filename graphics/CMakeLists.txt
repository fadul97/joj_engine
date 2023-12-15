# CMakeList.txt : CMake project for JojPlatform, include source and define
# project specific logic here.

cmake_minimum_required(VERSION 3.8)
project(JojGraphics)

# Build a library for D3D11 and D3D12
add_library(JojGraphics INTERFACE)

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

if(WIN32)
	# Include dx11 folder
	add_subdirectory(dx11)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dx11/)

	# Include dx12 folder
	add_subdirectory(dx12)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dx12/)

	target_link_libraries(JojGraphics INTERFACE JojGraphicsD3D11 JojGraphicsD3D12)
endif()