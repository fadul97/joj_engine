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

    // Specify the layout of the vertex(pos) data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (GLvoid*)0);

    // Specify the layout of the vertex(color) data
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (GLvoid*)(3 * sizeof(f32)));



    // Unbind the vbo and the vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Ignore back faces
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Wireframes
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void GLApp::init()
{
    // Geometries
    geo = JojRenderer::Cube(3.0f, 3.0f, 3.0f);
    //geo = JojRenderer::Cylinder(1.0f, 0.5f, 10.0f, 20, 10);
    //geo = JojRenderer::Sphere(1.0f, 40, 40);
    //geo = JojRenderer::GeoSphere(1.0f, 3);
    //geo = JojRenderer::Grid(100.0f, 20.0f, 20, 20);
    //geo = JojRenderer::Quad(3.0f, 1.0f);

    build_buffers();

    //shader = JojRenderer::Shader{ vshader_path , vfrag_path };
    shader.compile_shaders(geo_vertex, geo_frag);
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

    // controla rotação do cubo
    theta = DirectX::XM_PIDIV4;
    phi = DirectX::XM_PIDIV4;
    radius = 10.0f;

    // pega última posição do mouse
    last_xmouse = (f32)input->get_xmouse();
    last_ymouse = (f32)input->get_ymouse();

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
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
    DirectX::XMMATRIX Ry = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(30));
    DirectX::XMMATRIX Rx = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(-30));
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(0, 0, 0);
    DirectX::XMMATRIX W = S * Ry * Rx * T;

    // View Matrix
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(0, 0, -6, 1);
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
    DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(pos, target, up);

    // Projection Matrix
    DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45),
        JojEngine::Engine::pm->get_window()->get_aspect_ratio(),
        1.0f, 100.0f);

    // Word-View-Projection Matrix
    DirectX::XMMATRIX WorldViewProj = W * V * P;

    //shader.set_dxmat4("transform", WorldViewProj);
    
    DirectX::XMMATRIX mt {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };

    shader.set_dxmat4("transform", WorldViewProj);
}


b8 is_ortho = false;
f32 angle = 0.0f;
void GLApp::update()
{
    // Exit with ESCAPE key
	if (JojEngine::Engine::pm->is_key_pressed(VK_ESCAPE))
		JojEngine::Engine::close_engine();

    f32 xmouse = (f32)input->get_xmouse();
    f32 ymouse = (f32)input->get_ymouse();

    if (input->is_key_down(VK_LBUTTON))
    {
        // cada pixel corresponde a 1/4 de grau
        f32 dx = DirectX::XMConvertToRadians(0.4f * (xmouse - last_xmouse));
        f32 dy = DirectX::XMConvertToRadians(0.4f * (ymouse - last_ymouse));

        // atualiza ângulos com base no deslocamento do mouse 
        // para orbitar a câmera ao redor da caixa
        theta += dx;
        phi += dy;

        // restringe o ângulo de phi ]0-180[ graus
        phi = phi < 0.1f ? 0.1f : (phi > (DirectX::XM_PI - 0.1f) ? DirectX::XM_PI - 0.1f : phi);
    }
    else if (input->is_key_down(VK_RBUTTON))
    {
        // cada pixel corresponde a 0.05 unidades
        f32 dx = 0.05f * (xmouse - last_xmouse);
        f32 dy = 0.05f * (ymouse - last_ymouse);

        // atualiza o raio da câmera com base no deslocamento do mouse 
        radius += dx - dy;

        // restringe o raio (3 a 15 unidades)
        radius = radius < 3.0f ? 3.0f : (radius > 15.0f ? 15.0f : radius);
    }

    last_xmouse = xmouse;
    last_ymouse = ymouse;

    // converte coordenadas esféricas para cartesianas
    f32 x = radius * sinf(phi) * cosf(theta);
    f32 z = radius * sinf(phi) * sinf(theta);
    f32 y = radius * cosf(phi);

    // constrói a matriz da câmera (view matrix)
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&View, view);

    // constrói matriz combinada (world x view x proj)
    DirectX::XMMATRIX world = XMLoadFloat4x4(&World);
    DirectX::XMMATRIX proj = XMLoadFloat4x4(&Proj);
    DirectX::XMMATRIX WorldViewProj = world * view * proj;

    shader.set_dxmat4("transform", WorldViewProj);
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