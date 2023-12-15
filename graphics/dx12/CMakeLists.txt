# CMakeList.txt : CMake project for JojGraphics, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojGraphics)

add_library(JojGraphics graphics_dx12.cpp)

# Link DXGI, D3D12 and JojPlatform to JojGraphics
target_link_libraries(JojGraphics PRIVATE dxgi.lib d3d12.lib JojPlatform)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojGraphics PROPERTY CXX_STANDARD 20)
endif()
