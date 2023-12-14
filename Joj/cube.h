#pragma once

#include "game.h"
#include "renderer_dx12.h"

class Cube : public JojEngine::Game
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
	// Default members for handling pipeline
	ID3D12RootSignature* root_signature;
	ID3D12PipelineState* pipeline_state;
	
	// Buffers in CPU
	ID3DBlob* vertex_buffer_cpu;
	ID3DBlob* index_buffer_cpu;

	// Upload buffers: CPU -> GPU
	ID3D12Resource* vertex_buffer_upload;
	ID3D12Resource* index_buffer_upload;

	// Buffers in GPU
	ID3D12Resource* vertex_buffer_gpu;
	ID3D12Resource* index_buffer_gpu;

	// Buffer descriptors
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;	// Vertex buffer descriptor
	D3D12_INDEX_BUFFER_VIEW index_buffer_view;		// Index buffer descriptor

	// Vertex buffer characteristics
	u32 vertex_byte_stride;
	u32 vertex_buffer_size;

	// Index buffer characteristics
	DXGI_FORMAT index_format;
	u32 index_buffer_size;
};