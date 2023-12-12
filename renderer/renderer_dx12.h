#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "window_win32.h"
#include "graphics_dx12.h"
#include <DirectXColors.h>
#include <d3d12.h>

namespace JojRenderer
{
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};

	enum class AllocationType { GPU, UPLOAD };

	class DX12Renderer
	{
	public:
		DX12Renderer();
		~DX12Renderer();

		b8 init(JojPlatform::Win32Window* window, JojGraphics::DX12Graphics* graphics);	// Initialize renderer
		void draw();																	// Draw
		void present();																	// Present
		void clear(ID3D12PipelineState* pso);											// Clear backbuffer for next frame

		// Return Graphics Infrastructure
		ID3D12Device* get_device();		// Return graphics device

		void reset_commands();          // reinicia lista para receber novos comandos
		void submit_commands();			// Submit pending commands for execution

		// Allocate CPU memory to resource
		void allocate_resource_in_cpu(u32 size_in_bytes, ID3DBlob** resource);
		
		// Allocate GPU memory to resource
		void allocate_resource_in_gpu(AllocationType alloc_type, u32 size_in_bytes, ID3D12Resource** resource);
	
		// Copy vertices to Blob in CPU
		void copy_verts_to_cpu_blob(const void* vertices, u32 size_in_bytes, ID3DBlob* buffer_cpu);

		// Copy vertices to GPU
		void copy_verts_to_gpu(const void* vertices, u32 size_in_bytes, ID3D12Resource* buffer_upload, ID3D12Resource* buffer_gpu);

	private:
		JojPlatform::Win32Window* window;
		JojGraphics::DX12Graphics* graphics;

		// Configuration
		u32 backbuffer_count;							// Number of buffers on the swap chain (double, triple, etc.)
		b8 vsync;										// Vertical sync 
		f32 bg_color[4];								// Backbuffer background color

		// Graphics Infrastructure
		ID3D12Device* device;							// Graphics device
		IDXGIFactory6* factory;							// Main DXGI interface
		IDXGISwapChain* swapchain;						// Swap chain
		u32 backbuffer_index;							// Current backbuffer index

		// Pipeline
		ID3D12Resource** render_targets;				// Buffers for rendering (front and back)
		ID3D12Resource* depth_stencil;					// Depth & Stencil Buffer            
		ID3D12DescriptorHeap* render_target_heap;       // Descriptor heap for render targets
		ID3D12DescriptorHeap* depth_stencil_heap;       // Descriptor heap for Depth Stencil
		u32 rt_descriptor_size;							// Size of each Render Target descriptor
		D3D12_VIEWPORT viewport;						// Viewport
		D3D12_RECT scissor_rect;						// Scissor rect

		ID3D12CommandQueue* command_queue;              // GPU command queue
		ID3D12GraphicsCommandList* command_list;        // List of commands to submit to GPU
		ID3D12CommandAllocator* command_list_alloc;     // Memory used by the command list

		// CPU/GPU Synchronization
		ID3D12Fence* fence;								// Fence to synchronize CPU/GPU
		u64 current_fence;								// Fence counter

		b8 wait_command_queue();	// Wait for command queue execution
	};

	// Return graphics device
	inline ID3D12Device* DX12Renderer::get_device()
	{ return device; }
}

#endif  // PLATFORM_WINDOWS