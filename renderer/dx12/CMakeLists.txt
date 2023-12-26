# CMakeList.txt : CMake project for JojRendererD3D12, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojRendererD3D12)

add_library(JojRendererD3D12 renderer_dx12.cpp)

# Include renderer folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Link DXGI, D3D12 and JojPlatform to JojGraphics
target_link_libraries(JojRendererD3D12 PRIVATE dxgi.lib d3d12.lib JojPlatform)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojRendererD3D12 PROPERTY CXX_STANDARD 20)
endif()
