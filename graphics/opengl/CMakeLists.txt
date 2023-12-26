# CMakeList.txt : CMake project for JojGraphicsGL, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.8)
project(JojGraphicsGL)

add_library(JojGraphicsGL context_gl.cpp)

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojGraphicsGL PROPERTY CXX_STANDARD 20)
endif()

# Find the OpenGL package
find_package(OpenGL REQUIRED)

# Include the OpenGL headers directory
target_include_directories(JojGraphicsGL PRIVATE ${OPENGL_INCLUDE_DIR})

#target_include_directories(JojGraphicsGL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/GL/)

# Link OpenGL and JojPlatform to JojGraphicsGL
target_link_libraries(JojGraphicsGL PRIVATE ${OPENGL_gl_LIBRARY} JojPlatform)