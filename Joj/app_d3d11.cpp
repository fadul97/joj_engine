#include "app_d3d11.h"

#include "engine.h"
#include <d3dcompiler.h>
#include "error.h"

using namespace DirectX;


void D3D11App::init()
{
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
		window->get_aspect_ratio(),
		1.0f, 100.0f));

    // --------------------------------
    // Vertex Buffer
    // --------------------------------

	// Geometries
	geo = JojRenderer::Cube(3.0f, 3.0f, 3.0f);
	//geo = JojRenderer::Cylinder(1.0f, 0.5f, 3.0f, 20, 10);
	//geo = JojRenderer::Sphere(1.0f, 40, 40);
	//geo = JojRenderer::GeoSphere(1.0f, 3);
	//geo = JojRenderer::Grid(100.0f, 20.0f, 20, 20);
	//geo = JojRenderer::Quad(3.0f, 1.0f);

	// ------------------------------------------------------------------
	// ------->> Transformation, Visualization and Projection <<---------
	// ------------------------------------------------------------------

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
		window->get_aspect_ratio(),
		1.0f, 100.0f);

	// Word-View-Projection Matrix
	DirectX::XMMATRIX WorldViewProj = W * V * P;

	// --------------------------------
	// Constant Buffer
	// --------------------------------

	constBufferDesc.ByteWidth = sizeof(XMMATRIX);
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	XMMATRIX world_view_proj = XMMatrixTranspose(WorldViewProj);
	constantData.pSysMem = &world_view_proj;

	JojEngine::Engine::dx11_graphics->get_device()->CreateBuffer(&constBufferDesc, &constantData, &constant_buffer);

	JojEngine::Engine::dx11_graphics->get_context()->VSSetConstantBuffers(0, 1, &constant_buffer);

	// Create vertex buffer
	vertexBuffer = JojEngine::Engine::dx11_renderer->create_vertex_buffer(sizeof(Vertex), geo.get_vertex_count(), geo.get_vertex_data());

	// Create index buffer
	index_buffer = JojEngine::Engine::dx11_renderer->create_index_buffer(sizeof(u32), geo.get_index_count(), geo.get_index_data());

	DWORD shaderFlags = 0;
#ifndef _DEBUG
	shaderFlags |= D3D10_SHADER_DEBUG;						// Let compiler insert debug information into the output code
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;			// Compiler will not validate the generated code -> Recommended to use only with successfully compiled shaders
#endif // !_DEBUG

	// --------------------------------
	// Vertex Shader
	// --------------------------------

	// Compile and Create Vertex Shader
	ID3DBlob* vs_blob = nullptr;			// Vertex shader
	
	// Joj\--out\-build\-x64-debug\-joj\-Debug
	// Compile and create Vertex Shader
	vertex_shader = JojEngine::Engine::dx11_renderer->compile_and_create_vs_from_file(L"../../../../../joj/vertex.hlsl", vs_blob, shaderFlags);

	// Compile and Create Pixel Shader
	ID3DBlob* ps_blob = nullptr;			// Pixel shader
	pixel_shader = JojEngine::Engine::dx11_renderer->compile_and_create_ps_from_file(L"../../../../../joj/pixel.hlsl", ps_blob, shaderFlags);


	// --------------------------------
	// Input Assembler
	// --------------------------------

	// Input layout
	ID3D11InputLayout* input_layout = nullptr;

	// Description of Vertex Structure we created
	D3D11_INPUT_ELEMENT_DESC input_desc[2] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }				// 3 'floats' x 4 bytes = 12 bytes
	};

	// Create and bind input layout to Input Assembler Stage
	if (!JojEngine::Engine::dx11_renderer->create_and_set_input_layout(input_desc, ARRAYSIZE(input_desc), vs_blob, input_layout))
		OutputDebugString("Failed to create and set input layout\n");

	// Tell how Direct3D will form geometric primitives from vertex data
	JojEngine::Engine::dx11_graphics->get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ------------------------------------------------------------------------------------------------------------------------------------------------

	// Relase Direct3D resources
	vs_blob->Release();
	ps_blob->Release();
}

void D3D11App::update()
{
    // Exit with ESCAPE key
    if (input->is_key_press(VK_ESCAPE))
        window->close();

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

	// Update constant buffer with combined matrix (Word-View-Projection Matrix)
	JojRenderer::ObjectConstant obj_constant;
	XMStoreFloat4x4(&obj_constant.world_view_proj, DirectX::XMMatrixTranspose(WorldViewProj));
	constantData.pSysMem = &obj_constant.world_view_proj;

	JojEngine::Engine::dx11_graphics->get_device()->CreateBuffer(&constBufferDesc, &constantData, &constant_buffer);
	JojEngine::Engine::dx11_graphics->get_context()->VSSetConstantBuffers(0, 1, &constant_buffer);
}

void D3D11App::draw()
{
	JojEngine::Engine::dx11_renderer->clear();

	UINT stride = sizeof(Vertex);													// Store size os Vertex Structure
	UINT offset = 0;																// Pointer to where the first Vertex Buffer is in array

	// Bind Vertex Buffer to an input slot of the device
	JojEngine::Engine::dx11_graphics->get_context()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Bind index buffer to the pipeline
	JojEngine::Engine::dx11_graphics->get_context()->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

	// Bind Vertex and Pixel Shaders
	JojEngine::Engine::dx11_graphics->get_context()->VSSetShader(vertex_shader, nullptr, 0);
	JojEngine::Engine::dx11_graphics->get_context()->PSSetShader(pixel_shader, nullptr, 0);

	JojEngine::Engine::dx11_graphics->get_context()->VSSetConstantBuffers(0, 1, &constant_buffer);

	// Draw
	JojEngine::Engine::dx11_graphics->get_context()->DrawIndexedInstanced(geo.get_index_count(), 1, 0, 0, 0);
	//JojEngine::Engine::dx11_graphics->get_context()->DrawIndexed(geo.get_index_count(), 0, 0);


	JojEngine::Engine::dx11_renderer->swap_buffers();
}

void D3D11App::shutdown()
{
	if (constant_buffer)
		constant_buffer->Release();
}