#pragma once

#include "game.h"
#include "dx12/renderer_dx12.h"
#include "DirectXMath.h"
#include <d3d11.h>

#include "geometry.h"

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

class D3D11App : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

private:
	ID3D11Buffer* vertexBuffer = nullptr;		// Buffer resource
	ID3D11Buffer* index_buffer = nullptr;		// Index buffer;
	ID3D11VertexShader* vertexShader = nullptr;	// Manages Vertex Shade Program and control Vertex Shader Stage 
	ID3D11PixelShader* pixelShader = nullptr;	// Manages Pixel Shader Program and controls Pixel Shader Stage

	JojRenderer::Cube geo = {};
	ID3D11Buffer* constant_buffer = nullptr;

	ID3D11RasterizerState* raster_state = nullptr;	// Rasterizer state
};