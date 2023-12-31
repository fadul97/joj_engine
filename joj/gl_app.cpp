#include "gl_app.h"

#define JOJ_GL_DEFINE_EXTERN
#include "opengl/joj_gl.h"
#include "engine.h"
#include "logger.h"
#include <iostream>
#include "opengl/shader.h"

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

    shader.compile_shaders(geo_shader, fragmentShaderSource);
    shader.use();

    perspective = mat4_perspective(90.0f, JojEngine::Engine::pm->get_window()->get_aspect_ratio(), 0.1f, 100.0f);
    ortho = mat4_orthographic(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 100.0f);

    Mat4 scale = mat4_scale(mat4_identity(), vec3_create(0.8f, 0.8f, 0.8f));
    Mat4 t = mat4_mul(perspective, scale);

    shader.set_mat4(std::string{ "transform" }, t);
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
            shader.set_mat4(std::string{ "transform" }, t);
        }
        else
        {
            Mat4 t = mat4_mul(trans, perspective);
            shader.set_mat4(std::string{ "transform" }, t);
        }
    }
}

void GLApp::draw()
{
    
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    shader.use();
    glBindVertexArray(vao);         // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    

    JojEngine::Engine::pm->swap_buffers();
}

void GLApp::shutdown()
{
	
}