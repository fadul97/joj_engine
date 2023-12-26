# CMakeList.txt : CMake project for Joj, include source and define
# project specific logic here.

if(WIN32 AND MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
endif()

# Add source to this project's executable.
add_executable(${PROJECT_NAME} main.cpp Joj.h shapes.cpp app_d3d11.cpp gl_app.cpp)

# If Windows platform, setup entry point
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

# Include graphics folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../graphics/)

# Include renderer folder
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../renderer/)

# Link JojPlatform to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojPlatform)

# Link JojEngine to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojEngine)

# Link JojGraphics to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojGraphics)

# Link JojRenderer to Joj
target_link_libraries(${PROJECT_NAME} PRIVATE JojRenderer)

# TODO: Add tests and install targets if needed.

# Build HLSL shaders
add_custom_target (shaders)

set (HLSL_SHADER_FILES vertex.hlsl pixel.hlsl)
set_source_files_properties (vertex.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties (pixel.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties (${HLSL_SHADER_FILES} PROPERTIES ShaderModel "5_0")

foreach (FILE ${HLSL_SHADER_FILES})
  get_filename_component (FILE_WE ${FILE} NAME_WE)
  get_source_file_property (shadertype ${FILE} ShaderType)
  get_source_file_property (shadermodel ${FILE} ShaderModel)
  add_custom_command (TARGET shaders
    COMMAND fxc.exe /nologo /Emain /T${shadertype}_${shadermodel} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
    MAIN_DEPENDENCY ${FILE}
    COMMENT "HLSL ${FILE}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM)
endforeach (FILE)

# Main project
set (LIBS d3d11 D3DCompiler)
target_link_libraries (${PROJECT_NAME} PRIVATE ${LIBS})

add_dependencies (${PROJECT_NAME} shaders)



# Specify the source files
set(D3D11_SHADER_FILES
    VertexShader.hlsl
    PixelShader.hlsl
)

# Specify the destination directory (the same directory as the executable)
set(DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Copy shader files to the destination directory during CMake configuration
foreach(D3D11_SHADER_FILES ${SHADER_FILES})
    configure_file(${SHADER_FILE} ${DESTINATION_DIR} COPYONLY)
endforeach()