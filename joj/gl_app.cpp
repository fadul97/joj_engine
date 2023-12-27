#include "gl_app.h"

#define JOJ_GL_DEFINE_EXTERN
#include "opengl/joj_gl.h"
#include "engine.h"
#include "logger.h"
#include <iostream>

Mat4 perspective;
Mat4 ortho;
u32 uniform;

void GLApp::init()
{
    geo = JojRenderer::Cube(0.1f, 0.1f, 0.1f);
    //geo = JojRenderer::GeoSphere(0.2f, 3);

    float vertices[] = {
         0.5f,  0.5f, -1.0f,  // top right
         0.5f, -0.5f, -1.0f,  // bottom right
        -0.5f, -0.5f, -1.0f,  // bottom left
        -0.5f,  0.5f, -1.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    // Create a vbo and a vao
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    // Bind the vbo and the vao
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Fill the vbo with the vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Fill the ebo with the vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind the vbo and the vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create a vertex shader object
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Attach the vertex shader source code to the shader object
    glShaderSource(vertexShader, 1, &geo_shader, NULL);

    // Compile the vertex shader
    glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        FERROR(ERR_RENDERER, "ERROR::SHADER::VERTEX::COMPILATION_FAILED:");
        std::cout << infoLog << "\n";
    }

    // Create a fragment shader object
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Attach the fragment shader source code to the shader object
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // Compile the fragment shader
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        FERROR(ERR_RENDERER, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED:");
        std::cout << infoLog << "\n";
    }

    // Create a shader program object
    shader_program = glCreateProgram();

    // Attach the vertex and fragment shaders to the shader program
    glAttachShader(shader_program, vertexShader);
    glAttachShader(shader_program, fragmentShader);

    // Link the shader program
    glLinkProgram(shader_program);

    // Check for shader program linking errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        FERROR(ERR_RENDERER, "ERROR::SHADER::PROGRAM::COMPILATION_FAILED:");
        std::cout << infoLog << "\n";
    }

    // Delete the vertex and fragment shaders as they are no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Use the shader program
    glUseProgram(shader_program);
    
    perspective = mat4_perspective(90.0f, JojEngine::Engine::pm->get_window()->get_aspect_ratio(), 0.1f, 10.0f);
    ortho = mat4_orthographic(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 100.0f);

    Mat4 scale = mat4_scale(mat4_identity(), vec3_create(0.8f, 0.8f, 0.8f));
    Mat4 t = mat4_mul(perspective, scale);

    uniform = glGetUniformLocation(shader_program, "transform");
    if (uniform != -1)
    {
        glUniformMatrix4fv(uniform, 1, 0, t.data);
    }
    else
    {
        FERROR(ERR_RENDERER, "Failed to get uniform 'transform' location.");
    }
}   

b8 is_ortho = false;
f32 angle = 0.0f;
void GLApp::update()
{
    // Exit with ESCAPE key
	if (JojEngine::Engine::pm->is_key_pressed(VK_ESCAPE))
		JojEngine::Engine::close_engine();

    b8 update = false;
    if (JojEngine::Engine::pm->is_key_pressed('S'))
    {
        is_ortho = !is_ortho;
        update = true;
    }

    if (JojEngine::Engine::pm->is_key_down('D'))
    {
        angle += 0.01f;
        update = true;
    }
    if (JojEngine::Engine::pm->is_key_down('A'))
    {
        angle -= 0.01f;
        update = true;
    }

    if (update)
    {
        f32* m = perspective.data;

        if (is_ortho)
            m = ortho.data;

        Mat4 trans = mat4_euler_x(angle);

        if (is_ortho)
        {
            Mat4 t = mat4_mul(ortho, trans);
            glUniformMatrix4fv(uniform, 1, 0, t.data);
        }
        else
        {
            Mat4 t = mat4_mul(trans, perspective);
            glUniformMatrix4fv(uniform, 1, 0, t.data);
        }
    }
}

void GLApp::draw()
{
    
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    glUseProgram(shader_program);
    glBindVertexArray(vao);         // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    

    JojEngine::Engine::pm->swap_buffers();
}

void GLApp::shutdown()
{
	
}