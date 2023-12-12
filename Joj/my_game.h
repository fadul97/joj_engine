#pragma once

#include "game.h"
#include "renderer_dx12.h"

class MyGame : public JojEngine::Game
{
public:
	void init();
	void update();
	void display();
	void shutdown();
	void build_geometry();
	void build_root_signature();
	void build_pipeline_state();

private:
	ID3D12RootSignature* root_signature;
	ID3D12PipelineState* pipeline_state;

	// Geometry attributes
	u32 vertex_byte_stride;
	u32 vertex_buffer_size;

	// Buffer in CPU
	ID3DBlob* vertex_buffer_cpu;

	// Upload buffers: CPU -> GPU
	ID3D12Resource* vertex_buffer_upload;

	// Buffer in GPU
	ID3D12Resource* vertex_buffer_gpu;

	// Vertex buffer descriptor
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
};