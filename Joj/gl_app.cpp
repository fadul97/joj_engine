#include "gl_app.h"

#include "opengl/context_gl.h"
#include "engine.h"

void GLApp::init()
{
    // Define the vertex data for the triangle
    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f, // Left
         0.5f, -0.5f, 0.0f, // Right
         0.0f,  0.5f, 0.0f  // Top
    };


    // Create a vbo and a vao
    JojGraphics::GLContext::glGenBuffers(1, &vbo);
    JojGraphics::GLContext::glGenVertexArrays(1, &vao);

    // Bind the vbo and the vao
    JojGraphics::GLContext::glBindVertexArray(vao);
    JojGraphics::GLContext::glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Fill the vbo with the vertex data
    JojGraphics::GLContext::glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    JojGraphics::GLContext::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    JojGraphics::GLContext::glEnableVertexAttribArray(0);

    // Unbind the vbo and the vao
    JojGraphics::GLContext::glBindBuffer(GL_ARRAY_BUFFER, 0);
    JojGraphics::GLContext::glBindVertexArray(0);

    // Create a vertex shader object
    GLuint vertexShader = JojGraphics::GLContext::glCreateShader(GL_VERTEX_SHADER);

    // Attach the vertex shader source code to the shader object
    JojGraphics::GLContext::glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // Compile the vertex shader
    JojGraphics::GLContext::glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    GLint success;
    char infoLog[512];
    JojGraphics::GLContext::glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        JojGraphics::GLContext::glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        OutputDebugString("ERROR::SHADER::VERTEX::COMPILATION_FAILED:\n");
        OutputDebugString(infoLog);
    }

    // Create a fragment shader object
    GLuint fragmentShader = JojGraphics::GLContext::glCreateShader(GL_FRAGMENT_SHADER);

    // Attach the fragment shader source code to the shader object
    JojGraphics::GLContext::glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // Compile the fragment shader
    JojGraphics::GLContext::glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    JojGraphics::GLContext::glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        JojGraphics::GLContext::glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        OutputDebugString("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED:\n");
        OutputDebugString(infoLog);
    }

    // Create a shader program object
    shader_program = JojGraphics::GLContext::glCreateProgram();

    // Attach the vertex and fragment shaders to the shader program
    JojGraphics::GLContext::glAttachShader(shader_program, vertexShader);
    JojGraphics::GLContext::glAttachShader(shader_program, fragmentShader);

    // Link the shader program
    JojGraphics::GLContext::glLinkProgram(shader_program);

    // Check for shader program linking errors
    JojGraphics::GLContext::glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        JojGraphics::GLContext::glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        OutputDebugString("ERROR::SHADER::PROGRAM::COMPILATION_FAILED:\n");
        OutputDebugString(infoLog);
    }

    // Delete the vertex and fragment shaders as they are no longer needed
    JojGraphics::GLContext::glDeleteShader(vertexShader);
    JojGraphics::GLContext::glDeleteShader(fragmentShader);

    // Use the shader program
    JojGraphics::GLContext::glUseProgram(shader_program);
}

void GLApp::update()
{
    // Exit with ESCAPE key
	if (JojEngine::Engine::pm->is_key_pressed(VK_ESCAPE))
		JojEngine::Engine::close_engine();
}

void GLApp::draw()
{
    JojGraphics::GLContext::glClear(GL_COLOR_BUFFER_BIT);
    JojGraphics::GLContext::glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    JojGraphics::GLContext::glUseProgram(shader_program);
    JojGraphics::GLContext::glBindVertexArray(vao);         // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    JojGraphics::GLContext::glDrawArrays(GL_TRIANGLES, 0, 3);

    JojEngine::Engine::pm->swap_buffers();
}

void GLApp::shutdown()
{
	
}