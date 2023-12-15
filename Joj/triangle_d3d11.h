#pragma once

#include "game.h"
#include "dx12/renderer_dx12.h"
#include "DirectXMath.h"
#include <d3d11.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

class D3D11Triangle : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

private:
	ID3D11Buffer* vertexBuffer = nullptr;		// Buffer resource
	ID3D11VertexShader* vertexShader = nullptr;	// Manages Vertex Shade Program and control Vertex Shader Stage 
	ID3D11PixelShader* pixelShader = nullptr;	// Manages Pixel Shader Program and controls Pixel Shader Stage
};