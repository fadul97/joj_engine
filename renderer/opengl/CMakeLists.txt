# CMakeList.txt : CMake project for JojRendererD3D12, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojRendererGL)

add_library(JojRendererGL renderer_gl.cpp shader.cpp)

# Include renderer folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojRendererGL PROPERTY CXX_STANDARD 20)
endif()

# Find the OpenGL package
find_package(OpenGL REQUIRED)

# Include the OpenGL headers directory
target_include_directories(JojRendererGL PRIVATE ${OPENGL_INCLUDE_DIR})

# Link OpenGL and JojPlatform to JojRendererGL
target_link_libraries(JojRendererGL PRIVATE ${OPENGL_LIBRARY} JojPlatform)