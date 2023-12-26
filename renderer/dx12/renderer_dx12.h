#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "renderer.h"
#include "dx12/context_dx12.h"
#include <DirectXColors.h>
#include <d3d12.h>

namespace JojRenderer
{
	struct ObjectConstant
	{
		DirectX::XMFLOAT4X4 world_view_proj =
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	};

	enum class AllocationType { GPU, UPLOAD };

	class DX12Renderer : public Renderer
	{
	public:
		DX12Renderer();
		~DX12Renderer();

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize renderer
		void render();											// Draw to screen
		void clear();											// Clear screen
		void swap_buffers();									// Change front and back buffers
		void shutdown();										// Clear resources

		void custom_clear(ID3D12PipelineState* pso);			// Clear backbuffer for next frame

		// Return Graphics Infrastructure
		ID3D12Device* get_device();		// Return graphics device
		u32 get_antialiasing();			// Return number of samples for each pixel on the screen
		u32 get_quality();				// Return antialiasing sampling quality

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

		ID3D12CommandQueue* get_command_queue();			// Return GPU command queue
		ID3D12GraphicsCommandList* get_command_list();      // Return list of commands to submit to GPU
		ID3D12CommandAllocator* get_command_list_alloc();   // Return memory used by the command list

	private:
		std::unique_ptr<JojGraphics::DX12Context> context;

		// Configuration
		u32 backbuffer_count;							// Number of buffers on the swap chain (double, triple, etc.)
		u32 antialiasing;								// Number of samples for each pixel on the screen
		u32 quality;									// Antialiasing sampling quality
		b8 vsync;										// Vertical sync 
		f32 bg_color[4];								// Backbuffer background color

		// Graphics Infrastructure
		ID3D12Device* device;							// Graphics device
		IDXGIFactory6* factory;							// Main DXGI interface
		IDXGISwapChain1* swapchain;						// Swap chain
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

	// Return number of samples for each pixel on the screen
	inline u32 DX12Renderer::get_antialiasing()
	{ return antialiasing; }

	// Return antialiasing sampling quality
	inline u32 DX12Renderer::get_quality()
	{ return quality; }

	// Return GPU command queue
	inline ID3D12CommandQueue* DX12Renderer::get_command_queue()
	{ return command_queue; }

	// Return list of commands to submit to GPU
	inline ID3D12GraphicsCommandList* DX12Renderer::get_command_list()
	{ return command_list; }

	// Return memory used by the command list
	inline ID3D12CommandAllocator* DX12Renderer::get_command_list_alloc()
	{ return command_list_alloc; }
}

#endif  // PLATFORM_WINDOWS