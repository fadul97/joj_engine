﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)

# Setting up variable names
set(PROJECT_NAME "Joj")
set(PROJECT_VERSION 0.0.0)

# Set C++ version required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable Hot Reload for MSVC compilers if supported. - Default from Visual Studio
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Define project
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

# Include sub-projects
add_subdirectory(platform)
add_subdirectory(graphics)
add_subdirectory(renderer)
add_subdirectory(engine)
add_subdirectory(joj)
