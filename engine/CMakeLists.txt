cmake_minimum_required(VERSION 3.8)
project(JojEngine)

add_library(JojEngine engine.cpp game.cpp error.h)

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JojEngine PROPERTY CXX_STANDARD 20)
endif()

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

# Link JojEngine to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojPlatform)