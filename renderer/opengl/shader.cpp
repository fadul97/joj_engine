#include "shader.h"

#if PLATFORM_WINDOWS

#include "logger.h"
#include <fstream>
#include <sstream>
#include <iostream>

JojRenderer::Shader::Shader()
{
    id = 0;
}

JojRenderer::Shader::Shader(const char* vertex_path, const char* fragment_path)
{
    // 1. retrieve the vertex/fragment source code from filePath
    // Open Vertex Shader file
    std::ifstream vshader_file;
    vshader_file.open(vertex_path);
    if (vshader_file.fail())
    {
        FERROR(ERR_RENDERER, "Failed to open vertex shader file.");
    }

    // Open Fragment Shader file
    std::ifstream fshader_file;
    fshader_file.open(fragment_path);
    if (fshader_file.fail())
    {
        FERROR(ERR_RENDERER, "Failed to open fragment shader file.");
    }


    // Read file's buffer contents into streams
    std::stringstream vshader_stream, fshader_stream;
    vshader_stream << vshader_file.rdbuf();
    fshader_stream << fshader_file.rdbuf();

    // Close file handlers
    vshader_file.close();
    fshader_file.close();

    // Convert stream into string
    std::string vertex_code = vshader_stream.str();
    std::string fragment_code = fshader_stream.str();

    const char* vshader_code = vertex_code.c_str();
    const char* fshader_code = fragment_code.c_str();

    // 2. Compile shaders
    u32 vertex;
    u32 fragment;

    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vshader_code, NULL);
    glCompileShader(vertex);
    check_compile_errors(vertex, ShaderType::VERTEX);

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fshader_code, NULL);
    glCompileShader(fragment);
    check_compile_errors(fragment, ShaderType::FRAGMENT);

    // Shader Program
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    check_compile_errors(id, ShaderType::PROGRAM);
    
    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

JojRenderer::Shader::~Shader()
{
}

void JojRenderer::Shader::compile_shaders(const char* vertex_shader, const char* fragment_shader)
{
    // 1. Compile shaders
    u32 vertex;
    u32 fragment;

    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_shader, NULL);
    glCompileShader(vertex);
    check_compile_errors(vertex, ShaderType::VERTEX);

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_shader, NULL);
    glCompileShader(fragment);
    check_compile_errors(fragment, ShaderType::FRAGMENT);

    // Shader Program
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    check_compile_errors(id, ShaderType::PROGRAM);

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    FDEBUG("Shaders compiled!");
}

void JojRenderer::Shader::check_compile_errors(u32 shader, ShaderType type)
{   
    std::string shadertype_name;
    switch (type)
    {
    case ShaderType::VERTEX:
        shadertype_name = std::string{ "VERTEX" };
        break;

    case ShaderType::FRAGMENT:
        shadertype_name = std::string{ "FRAGMENT" };
        break;

    case ShaderType::PROGRAM:
        shadertype_name = std::string{ "PROGRAM" };
        break;
    }

    i32 success;
    char info_log[1024];
    
    if (type != ShaderType::PROGRAM)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, info_log);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << shadertype_name << "\n" << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
            //FERROR(ERR_RENDERER, "ERROR::SHADER_COMPILATION_ERROR of type : %s.\n%s\n", shadertype_name.c_str(), info_log);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, info_log);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << shadertype_name << "\n" << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
            //FERROR(ERR_RENDERER, "ERROR::PROGRAM_LINKING_ERROR of type : %s.\n%s\n", shadertype_name.c_str(), info_log);
        }
    }
}

#endif PLATFORM_WINDOWS