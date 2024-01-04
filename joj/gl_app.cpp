#include "gl_app.h"

#define JOJ_GL_DEFINE_EXTERN
#include "opengl/joj_gl.h"
#include "engine.h"
#include "logger.h"
#include <iostream>
#include "opengl/shader.h"
#include <fstream>

Mat4 perspective;
Mat4 ortho;
u32 uniform;

void GLApp::build_buffers()
{
    DirectX::XMFLOAT3 vertices[] = {
        DirectX::XMFLOAT3{ +0.5f, +0.5f, +0.0f },  // top right
        DirectX::XMFLOAT3{ +0.5f, -0.5f, +0.0f },  // bottom right
        DirectX::XMFLOAT3{ -0.5f, -0.5f, +0.0f },  // bottom left
        DirectX::XMFLOAT3{ -0.5f, +0.5f, +0.0f }   // top left 
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
    glBufferData(GL_ARRAY_BUFFER, (geo.get_vertex_count() * sizeof(JojRenderer::Vertex)), geo.get_vertex_data(), GL_STATIC_DRAW);

    // Fill the ebo with the vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geo.get_index_count() * sizeof(u32), geo.get_index_data(), GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 7 * sizeof(f32), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind the vbo and the vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GLApp::init()
{
    // Geometries
    //geo = JojRenderer::Cube(1.0f, 1.0f, 1.0f);
    //geo = JojRenderer::Cylinder(1.0f, 0.5f, 0.7f, 20, 10);
    //geo = JojRenderer::Sphere(1.0f, 40, 40);
    //geo = JojRenderer::GeoSphere(1.0f, 3);
    //geo = JojRenderer::Grid(100.0f, 20.0f, 20, 20);
    geo = JojRenderer::Quad(3.0f, 1.0f);

    build_buffers();

    //shader = JojRenderer::Shader{ vshader_path , vfrag_path };
    shader.compile_shaders(vertexShaderSource, fragmentShaderSource);
    shader.use();

    std::ifstream file("../shaders/frag.glsl");

    if (!file) {
        std::cerr << "Unable to open file frag.glsl";
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << '\n';
    }

    file.close();

    perspective = mat4_perspective(90.0f, JojEngine::Engine::pm->get_window()->get_aspect_ratio(), 0.1f, 10.0f);
    ortho = mat4_orthographic(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 100.0f);
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
            shader.set_mat4("transform", t);
            mat4_print(t);
        }
        else
        {
            Mat4 t = mat4_mul(trans, perspective);
            shader.set_mat4("transform", t);
            mat4_print(t);
        }
    }
}

void GLApp::draw()
{
    
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);

    shader.use();
    glBindVertexArray(vao);         // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, geo.get_index_count(), GL_UNSIGNED_INT, 0);
    

    JojEngine::Engine::pm->swap_buffers();
}

void GLApp::shutdown()
{
	
}