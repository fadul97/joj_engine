# CMakeList.txt : CMake project for JojRendererD3D11, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojRendererD3D11)

add_library(JojRendererD3D11 renderer_dx11.cpp)

# Link DXGI, D3D11 and JojPlatform to JojGraphics
target_link_libraries(JojRendererD3D11 PRIVATE dxgi.lib d3d11.lib JojPlatform)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojRendererD3D11 PROPERTY CXX_STANDARD 20)
endif()
