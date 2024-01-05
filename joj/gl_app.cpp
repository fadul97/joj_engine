#include "gl_app.h"

#define JOJ_GL_DEFINE_EXTERN
#include "opengl/joj_gl.h"
#include "engine.h"
#include "logger.h"
#include "opengl/shader.h"
#include <fstream>

#include <iostream>
using namespace std;

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

    // Specify the layout of the vertex(pos) data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (GLvoid*)0);

    // Specify the layout of the vertex(color) data
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (GLvoid*)(3 * sizeof(f32)));

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    //glGenVertexArrays(1, &light_vao);
    //glBindVertexArray(light_vao);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Unbind the vbo and the vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // -----------------------------------------------------------------------------------------------------------------------------------
    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    // Setup Light
    
    // Create a lvbo and a light_vao
    glGenBuffers(1, &lvbo);
    glGenBuffers(1, &lebo);
    glGenVertexArrays(1, &light_vao);

    // Bind the lvbo and the vao
    glBindVertexArray(light_vao);
    glBindBuffer(GL_ARRAY_BUFFER, lvbo);

    // Fill the lvbo with the vertex data
    glBufferData(GL_ARRAY_BUFFER, (geo.get_vertex_count() * sizeof(JojRenderer::Vertex)), geo.get_vertex_data(), GL_STATIC_DRAW);

    // Fill the lebo with the vertex data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geo.get_index_count() * sizeof(u32), geo.get_index_data(), GL_STATIC_DRAW);

    // Specify the layout of the vertex(pos) data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (GLvoid*)0);

    // Specify the layout of the vertex(color) data
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (GLvoid*)(3 * sizeof(f32)));

    // Unbind the lvbo and the vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // -----------------------------------------------------------------------------------------------------------------------------------



    // Ignore back faces
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    // Wireframes
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

f32 x = 0;
f32 y = 0;
f32 z = 20;
f32 velocity = 10.0f;

// lighting
DirectX::XMFLOAT3 lightPos{ 1.2f, 10.0f, 10.0f };
void GLApp::init()
{
    // Geometries
    geo = JojRenderer::Cube{ 3.0f, 3.0f, 3.0f, DirectX::XMFLOAT4{1.0f, 0.0f, 0.0f, 1.0f} };
    //light_cube.move_to(lightPos.x, lightPos.y, lightPos.z);

    build_buffers();

    shader.compile_shaders(geo_vertex, geo_frag);
    light_shader.compile_shaders(light_vertex, light_frag);

    // inicializa as matrizes World e View para a identidade
    World = View = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };

    // inicializa a matriz de projeção
    XMStoreFloat4x4(&Proj, DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        JojEngine::Engine::pm->get_window()->get_aspect_ratio(),
        1.0f, 100.0f));

    // World Matrix
    DirectX::XMMATRIX W = DirectX::XMMatrixIdentity();

    // View Matrix
    camera.position = DirectX::XMFLOAT3{ 0.64f, -0.56f, camera.position.z -3 };
    DirectX::XMMATRIX V = camera.get_view_mat();

    // Projection Matrix
    DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(camera.zoom),
        JojEngine::Engine::pm->get_window()->get_aspect_ratio(),
        1.0f, 100.0f);

    // Word-View-Projection Matrix
    DirectX::XMMATRIX WorldViewProj = W * V * P;

    shader.use();
    shader.set_dxmat4("transform", WorldViewProj);


    WorldViewProj = W * V * P;
    light_shader.use();
    light_shader.set_dxmat4("transform", WorldViewProj);

    mouse_callback(JojEngine::Engine::pm->get_xmouse(), JojEngine::Engine::pm->get_ymouse());

    JojEngine::Engine::pm->get_window()->hide_cursor(true);

    centerX = JojEngine::Engine::pm->get_window()->get_xcenter();
    centerY = JojEngine::Engine::pm->get_window()->get_ycenter();

    cmouseX = JojEngine::Engine::pm->get_xmouse();
    cmouseY = JojEngine::Engine::pm->get_ymouse();

    FDEBUG("%dx%d", centerX, centerY);
}


b8 is_ortho = false;
f32 angle = 0.0f;
bool firstPerson = true;
bool hideCursor = false;
void GLApp::update()
{
    // Exit with ESCAPE key
	if (JojEngine::Engine::pm->is_key_pressed(VK_ESCAPE))
		JojEngine::Engine::close_engine();

    if (JojEngine::Engine::pm->is_key_pressed(VK_TAB))
    {
        firstPerson = !firstPerson;
        hideCursor = !hideCursor;
        ShowCursor(hideCursor);
    }
    

    if (firstPerson)
    {
        // Now read the mouse position
        POINT cursorPos;
        GetCursorPos(&cursorPos);  // Get the current cursor position
        // Rotate freely inside window
        SetCursorPos(centerX, centerY);

        // Calculate the mouse movement
        int xoffset = cursorPos.x - centerX;
        int yoffset = centerY - cursorPos.y;

        //mouse_callback(JojEngine::Engine::pm->get_xmouse(), JojEngine::Engine::pm->get_ymouse());
        camera.process_mouse_movement(xoffset, yoffset);
        process_camera_input();
    }

    // Transformations
    DirectX::XMMATRIX world = XMLoadFloat4x4(&World);

    // constrói a matriz da câmera (view matrix)
    DirectX::XMMATRIX view = camera.get_view_mat();
    XMStoreFloat4x4(&View, view);

    // Projection Matrix
    DirectX::XMMATRIX proj = XMLoadFloat4x4(&Proj);

    // Word-View-Projection Matrix
    DirectX::XMMATRIX WorldViewProj = world * view * proj;

    shader.use();
    shader.set_dxmat4("transform", WorldViewProj);

    
    world = DirectX::XMMatrixIdentity();
    world = DirectX::XMMatrixTranslation(lightPos.x, lightPos.y, lightPos.z);
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f);
    world = DirectX::XMMatrixMultiply(world, scale);
    // Word-View-Projection Matrix
    WorldViewProj = world * view * proj;
    
    light_shader.use();
    light_shader.set_dxmat4("transform", WorldViewProj);
}

void GLApp::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

    // be sure to activate shader when setting uniforms/drawing objects
    light_shader.use();
    glBindVertexArray(light_vao);
    glDrawElements(GL_TRIANGLES, light_cube.get_index_count(), GL_UNSIGNED_INT, 0);

    shader.use();
    glBindVertexArray(vao);         // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    shader.set_vec3("objectColor", 1.0f, 0.5f, 0.31f);
    shader.set_vec3("lightColor", 1.0f, 1.0f, 1.0f);
    glDrawElements(GL_TRIANGLES, geo.get_index_count(), GL_UNSIGNED_INT, 0);
    

    JojEngine::Engine::pm->swap_buffers();
}

void GLApp::shutdown()
{
    ShowCursor(TRUE);
    ClipCursor(NULL);
}

void GLApp::process_camera_input()
{
    if (JojEngine::Engine::pm->is_key_down('W'))
    {
        camera.process_keyboard(JojRenderer::CameraMovement::FORWARD, JojEngine::Engine::frametime);
    }
    if (JojEngine::Engine::pm->is_key_down('S'))
    {
        camera.process_keyboard(JojRenderer::CameraMovement::BACKWARD, JojEngine::Engine::frametime);
    }

    if (JojEngine::Engine::pm->is_key_down('A'))
    {
        camera.process_keyboard(JojRenderer::CameraMovement::LEFT, JojEngine::Engine::frametime);
    }
    if (JojEngine::Engine::pm->is_key_down('D'))
    {
        camera.process_keyboard(JojRenderer::CameraMovement::RIGHT, JojEngine::Engine::frametime);
    }


}

void GLApp::mouse_callback(double xposIn, double yposIn)
{

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.process_mouse_movement(xoffset, yoffset);

    //SetCursorPos(centerX, centerY);
}