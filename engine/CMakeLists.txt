cmake_minimum_required(VERSION 3.8)
project(JojEngine)

add_library(JojEngine engine.cpp game.cpp error.cpp "logger.cpp" error_list.cpp logger.h "fmath.h")

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojEngine PROPERTY CXX_STANDARD 20)
endif()

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../graphics/)

# Include renderer folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../renderer/)

# Link JojEngine with JojPlatform and JojGraphics
target_link_libraries(JojEngine
	PRIVATE
	JojPlatform
	JojGraphics
	JojRenderer)