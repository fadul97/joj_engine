# CMakeList.txt : CMake project for Joj, include source and define
# project specific logic here.

# Add source to this project's executable.
add_executable(${PROJECT_NAME} "main.cpp" "Joj.h")

# If Window platform, setup entry point
if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:WINDOWS")
endif()

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# Include platform folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform/)

# Include engine folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/)

# Link JojPlatform to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojPlatform)

# Link JojEngine to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojEngine)

# TODO: Add tests and install targets if needed.
