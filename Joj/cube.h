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

	void build_constant_buffers();
	void build_geometry();
	void build_root_signature();
	void build_pipeline_state();

private:
	// Default members for handling pipeline
	ID3D12RootSignature* root_signature = nullptr;
	ID3D12PipelineState* pipeline_state = nullptr;
	
	// Buffers in CPU
	ID3DBlob* vertex_buffer_cpu = nullptr;
	ID3DBlob* index_buffer_cpu = nullptr;

	// Upload buffers: CPU -> GPU
	ID3D12Resource* vertex_buffer_upload = nullptr;
	ID3D12Resource* index_buffer_upload = nullptr;

	// Buffers in GPU
	ID3D12Resource* vertex_buffer_gpu = nullptr;
	ID3D12Resource* index_buffer_gpu = nullptr;

	// Buffer descriptors
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view = { 0 };	// Vertex buffer descriptor
	D3D12_INDEX_BUFFER_VIEW index_buffer_view = { 0 };		// Index buffer descriptor

	// Vertex buffer characteristics
	u32 vertex_byte_stride = 0;
	u32 vertex_buffer_size = 0;

	// Index buffer characteristics
	DXGI_FORMAT index_format = DXGI_FORMAT_UNKNOWN;
	u32 index_buffer_size = 0;

	// Constant buffer attributes
	ID3D12DescriptorHeap* constant_buffer_heap = nullptr;
	ID3D12Resource* constant_buffer_upload = nullptr;
	BYTE* constant_buffer_data = nullptr;
};