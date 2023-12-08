# CMakeList.txt : CMake project for Joj, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable(Joj "main.cpp" "Joj.h")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
if(WIN32)
	target_link_options(Joj PRIVATE "/SUBSYSTEM:WINDOWS")
endif()



if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Joj PROPERTY CXX_STANDARD 20)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform)

target_link_libraries(Joj PRIVATE JojPlatform)

# TODO: Add tests and install targets if needed.
