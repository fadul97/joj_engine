#include "app_d3d11.h"

#include "engine.h"
#include <d3dcompiler.h>
#include "error.h"

using namespace DirectX;


void D3D11App::init()
{
    // --------------------------------
    // Vertex Buffer
    // --------------------------------

	// Geometries
	//geo = JojRenderer::Cube(0.2f, 0.2f, 0.2f);

	// Geometry vertexes
	JojRenderer::Vertex vertices[8] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }
	};

	// Geometry indexes
	u16 indices[36] =
	{
		// front face
		0, 1, 3,
		1, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

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

	// Place vertices in the projection window
	for (int i = 0; i < 8; ++i)
	{
		DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices[i].pos);
		DirectX::XMVECTOR proj = DirectX::XMVector3TransformCoord(vertex, WorldViewProj);
		DirectX::XMStoreFloat3(&vertices[i].pos, proj);
	}

	// Describe Buffer - Resource structure
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(Vertex) * 8;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Set data we want to initialize the buffer contents with
	D3D11_SUBRESOURCE_DATA srd = { vertices, 0, 0};

	// Create Buffer
	if FAILED(JojEngine::Engine::dx11_graphics->get_device()->CreateBuffer(&bufferDesc, &srd, &vertexBuffer))
		MessageBoxA(nullptr, "Failed to create Buffer", 0, 0);

	// Describe index buffer
	D3D11_BUFFER_DESC index_buffer_desc;
	index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	index_buffer_desc.ByteWidth = 36 * sizeof(u16);
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	// Specify the data to initialize the index buffer.
	D3D11_SUBRESOURCE_DATA indices_init_data;
	indices_init_data.pSysMem = indices;

	// Create the index buffer.
	ThrowIfFailed(JojEngine::Engine::dx11_graphics->get_device()->CreateBuffer(&index_buffer_desc, &indices_init_data, &index_buffer));

	// Bind index buffer to the pipeline
	JojEngine::Engine::dx11_graphics->get_context()->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);

	DWORD shaderFlags = 0;
#ifndef _DEBUG
	shaderFlags |= D3D10_SHADER_DEBUG;						// Let compiler insert debug information into the output code
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;			// Compiler will not validate the generated code -> Recommended to use only with successfully compiled shaders
#endif // !_DEBUG

	// --------------------------------
	// Vertex Shader
	// --------------------------------

	// Compile and Create Vertex Shader
	ID3DBlob* shaderCompileErrorsBlob;						// To get info about compilation

	ID3DBlob* vsBlob;										// Vertex shader
	
	// Joj\--out\-build\-x64-debug\-joj\-Debug
	if FAILED(D3DCompileFromFile(L"../../../../../joj/VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", shaderFlags, NULL, &vsBlob, &shaderCompileErrorsBlob))
		MessageBoxA(nullptr, "Failed to compile Vertex Shader.", 0, 0);

	if FAILED(JojEngine::Engine::dx11_graphics->get_device()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader))
		MessageBoxA(nullptr, "Failed to create Vertex Shader.", 0, 0);


	// Compile and Create Pixel Shader
	ID3DBlob* psBlob;										// Pixel shader
	if FAILED(D3DCompileFromFile(L"../../../../../joj/PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", shaderFlags, NULL, &psBlob, &shaderCompileErrorsBlob))
		MessageBoxA(nullptr, "Failed to compile Pixel Shader.", 0, 0);

	if FAILED(JojEngine::Engine::dx11_graphics->get_device()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader))
		MessageBoxA(nullptr, "Failed to create Pixel Shader.", 0, 0);

	// --------------------------------
	// Input Assembler
	// --------------------------------

	// Input layout
	ID3D11InputLayout* inputLayout;

	// Description of Vertex Structure we created
	D3D11_INPUT_ELEMENT_DESC inputDesc[2] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }				// 3 'floats' x 4 bytes = 12 bytes
	};

	// Create input layout
	ThrowIfFailed(JojEngine::Engine::dx11_graphics->get_device()->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout));

	// Bind input layout to the Imput Assembler Stage
	JojEngine::Engine::dx11_graphics->get_context()->IASetInputLayout(inputLayout);

	// Tell how Direct3D will form geometric primitives from vertex data
	JojEngine::Engine::dx11_graphics->get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ------------------------------------------------------------------------------------------------------------------------------------------------

	// Relase Direct3D resources
	psBlob->Release();
	vsBlob->Release();


	// --------------------
	// ---- Rasterizer ----
	// --------------------

	// TODO: comment specifications on rasterizer
	// Describe rasterizer
	D3D11_RASTERIZER_DESC rasterizer = {};
	//rasterizer.FillMode = D3D11_FILL_SOLID;
	rasterizer.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer.CullMode = D3D11_CULL_BACK;
	//rasterizer.CullMode = D3D11_CULL_NONE;
	rasterizer.FrontCounterClockwise = FALSE;
	rasterizer.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	rasterizer.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizer.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizer.DepthClipEnable = TRUE;
	rasterizer.MultisampleEnable = FALSE;
	rasterizer.AntialiasedLineEnable = FALSE;
}

void D3D11App::update()
{
    // Exit with ESCAPE key
    if (input->is_key_press(VK_ESCAPE))
        window->close();
}

void D3D11App::draw()
{
	JojEngine::Engine::dx11_renderer->clear();

	UINT stride = sizeof(Vertex);													// Store size os Vertex Structure
	UINT offset = 0;																// Pointer to where the first Vertex Buffer is in array

	// Bind Vertex Buffer to an input slot of the device
	JojEngine::Engine::dx11_graphics->get_context()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Bind index buffer to the pipeline
	JojEngine::Engine::dx11_graphics->get_context()->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);

	// Bind Vertex and Pixel Shaders
	JojEngine::Engine::dx11_graphics->get_context()->VSSetShader(vertexShader, nullptr, 0);
	JojEngine::Engine::dx11_graphics->get_context()->PSSetShader(pixelShader, nullptr, 0);

	JojEngine::Engine::dx11_graphics->get_context()->VSSetConstantBuffers(0, 1, &pConstantBuffer);

	// Draw
	u32 num_indices = 36;
	JojEngine::Engine::dx11_graphics->get_context()->DrawIndexedInstanced(num_indices, 1, 0, 0, 0);

	JojEngine::Engine::dx11_renderer->present();
}

void D3D11App::shutdown()
{

}