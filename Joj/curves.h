#pragma once

#include "game.h"
#include "renderer_dx12.h"

class Curves : public JojEngine::Game
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
	static const u32 max_vertices = 100;
	JojRenderer::Vertex vertices[max_vertices];
	JojRenderer::Vertex v[max_vertices];
	u32 count;
	u32 index;


	// Default members for handling pipeline
	ID3D12RootSignature* root_signature;
	ID3D12PipelineState* pipeline_state;

	// Default members for handling geometry
	u32 vertex_byte_stride;
	u32 vertex_buffer_size;
	ID3DBlob* vertex_buffer_cpu;					// Buffer in CPU
	ID3D12Resource* vertex_buffer_upload;			// Upload buffers: CPU -> GPU
	ID3D12Resource* vertex_buffer_gpu;				// Buffer in GPU
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;	// Vertex buffer descriptor
};