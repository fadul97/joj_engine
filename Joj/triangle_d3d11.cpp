#include "triangle_d3d11.h"

#include "engine.h"
#include <d3dcompiler.h>
#include "error.h"


void D3D11Triangle::init()
{
    // --------------------------------
    // Vertex Buffer
    // --------------------------------

	// Set vertices
	Vertex vertices[3] =
	{
		{ DirectX::XMFLOAT3(+0.0f, +0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
		{ DirectX::XMFLOAT3(+0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Purple) },
	};

	// Describe Buffer - Resource structure
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Set data we want to initialize the buffer contents with
	D3D11_SUBRESOURCE_DATA srd = { vertices, 0, 0 };


	// Create Buffer
	if FAILED(JojEngine::Engine::dx11_graphics->get_device()->CreateBuffer(&bufferDesc, &srd, &vertexBuffer))
		MessageBoxA(nullptr, "Failed to create Buffer", 0, 0);

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

}

void D3D11Triangle::update()
{
    // Exit with ESCAPE key
    if (input->is_key_press(VK_ESCAPE))
        window->close();
}

void D3D11Triangle::draw()
{
	JojEngine::Engine::dx11_renderer->clear();

	UINT stride = sizeof(Vertex);													// Store size os Vertex Structure
	UINT offset = 0;																// Pointer to where the first Vertex Buffer is in array

	// Bind Vertex Buffer to an input slot of the device
	JojEngine::Engine::dx11_graphics->get_context()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Bind Vertex and Pixel Shaders
	JojEngine::Engine::dx11_graphics->get_context()->VSSetShader(vertexShader, nullptr, 0);
	JojEngine::Engine::dx11_graphics->get_context()->PSSetShader(pixelShader, nullptr, 0);

	// Draw
	UINT numVerts = 3;
	JojEngine::Engine::dx11_graphics->get_context()->Draw(numVerts, 0);

	JojEngine::Engine::dx11_renderer->present();
}

void D3D11Triangle::shutdown()
{

}