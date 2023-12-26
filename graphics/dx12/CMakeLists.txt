# CMakeList.txt : CMake project for JojGraphics, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojGraphicsD3D12)

add_library(JojGraphicsD3D12 context_dx12.cpp)

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Link DXGI, D3D12 and JojPlatform to JojGraphics
target_link_libraries(JojGraphicsD3D12 PRIVATE dxgi.lib d3d12.lib dxguid.lib JojPlatform)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojGraphicsD3D12 PROPERTY CXX_STANDARD 20)
endif()
