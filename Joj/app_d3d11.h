#pragma once

#include "game.h"
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
	ID3D11VertexShader* vertex_shader = nullptr;	// Manages Vertex Shade Program and control Vertex Shader Stage 
	ID3D11PixelShader* pixel_shader = nullptr;	// Manages Pixel Shader Program and controls Pixel Shader Stage

	ID3D11Buffer* constant_buffer = nullptr;
	D3D11_SUBRESOURCE_DATA constantData = { 0 };
	D3D11_BUFFER_DESC constBufferDesc = { 0 };

	//JojRenderer::Cube geo = {};
	//JojRenderer::Cylinder geo = {};
	//JojRenderer::Sphere geo = {};
	JojRenderer::GeoSphere geo = {};
	//JojRenderer::Grid geo = {};
	//JojRenderer::Quad geo = {};

	ID3D11RasterizerState* raster_state = nullptr;	// Rasterizer state

	// Camera settings
	DirectX::XMFLOAT4X4 World = {};
	DirectX::XMFLOAT4X4 View = {};
	DirectX::XMFLOAT4X4 Proj = {};

	f32 theta = 0;
	f32 phi = 0;
	f32 radius = 0;

	f32 last_xmouse = 0;
	f32 last_ymouse = 0;
};