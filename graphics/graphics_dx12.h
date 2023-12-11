#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <dxgi1_6.h>    // DirectX graphics infrastructure
#include <d3d12.h>      // Main Direct3D functions
#include "window_win32.h"

namespace JojGraphics
{
	class DX12Graphics
	{
	public:
		DX12Graphics();
		~DX12Graphics();

		void init(JojPlatform::Win32Window* window);	// Initialize Direct3D

		void draw();									// Draw
		void present();									// Present
		void clear(ID3D12PipelineState* pso);			// Clear backbuffer for next frame

		// Configuration get methods
		u32 get_backbuffer_count();						// Return number of buffers on the swap chain (double, triple, etc.)
		b8 get_vsync();									// Return vertical sync 
		f32* get_bg_color();							// Return backbuffer background color

		// Graphics Infrastructure get methods
		ID3D12Device4* get_device();					// Return graphics device
		IDXGIFactory6* get_factory();					// Return DXGI interface
		IDXGISwapChain1* get_swapchain();				// Return swap chain

		// Pipeline get methods
		ID3D12Resource** get_render_targets();				// Return buffers for rendering (front and back)
		ID3D12Resource* get_depth_stencil();				// Return Depth & Stencil Buffer            
		ID3D12DescriptorHeap* get_render_target_heap();     // Return descriptor heap for render targets
		ID3D12DescriptorHeap* get_depth_stencil_heap();     // Return descriptor heap for Depth Stencil
		u32 get_rt_descriptor_size();						// Return size of each Render Target descriptor
		
		D3D12_VIEWPORT& get_viewport();						// Return viewport
		D3D12_RECT& get_scissor_rect();						// Return scissor rect

		ID3D12CommandQueue* get_command_queue();			// Return GPU command queue
		ID3D12GraphicsCommandList* get_command_list();      // Return list of commands to submit to GPU
		ID3D12CommandAllocator* get_command_list_alloc();   // Return memory used by the command list

		// CPU/GPU Synchronization get methods
		ID3D12Fence* get_fence();							// Return fence to synchronize CPU/GPU
		u64 get_current_fence();							// Return fence counter

	private:
		// Configuration
		u32 backbuffer_count;							// Number of buffers on the swap chain (double, triple, etc.)
		u32 antialiasing;								// Number of samples for each pixel on the screen
		u32 quality;									// Antialiasing sampling quality
		b8 vsync;										// Vertical sync 
		f32 bg_color[4];								// Backbuffer background color

		// Graphics Infrastructure
		ID3D12Device4* device;							// Graphics device
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

		void log_hardware_info();	// Show hardware information
		b8 wait_command_queue();	// Wait for command queue execution
		void submit_commands();		// Submit pending commands for execution
	};

	// Return number of buffers on the swap chain (double, triple, etc.)
	inline u32 DX12Graphics::get_backbuffer_count()
	{ return backbuffer_count; }

	// Return vertical sync 
	inline b8 DX12Graphics::get_vsync()
	{ return vsync; }

	// Return backbuffer background color
	inline f32* DX12Graphics::get_bg_color()
	{ return bg_color; }

	// Return graphics device
	inline ID3D12Device4* DX12Graphics::get_device()
	{ return device; }

	// Return DXGI interface
	inline IDXGIFactory6* DX12Graphics::get_factory()
	{ return factory; }

	// Return swap chain
	inline IDXGISwapChain1* DX12Graphics::get_swapchain()
	{ return swapchain; }

	// Return buffers for rendering (front and back)
	inline ID3D12Resource** DX12Graphics::get_render_targets()
	{ return render_targets; }

	// Return Depth & Stencil Buffer 
	inline ID3D12Resource* DX12Graphics::get_depth_stencil()
	{ return depth_stencil; }

	// Return descriptor heap for render targets
	inline ID3D12DescriptorHeap* DX12Graphics::get_render_target_heap()
	{ return render_target_heap; }

	// Return descriptor heap for Depth Stencil
	inline ID3D12DescriptorHeap* DX12Graphics::get_depth_stencil_heap()
	{ return depth_stencil_heap; }

	// Return size of each Render Target descriptor
	inline u32 DX12Graphics::get_rt_descriptor_size()
	{ return rt_descriptor_size; }

	// Return viewport
	inline D3D12_VIEWPORT& DX12Graphics::get_viewport()
	{ return viewport; }

	// Return scissor rect
	inline D3D12_RECT& DX12Graphics::get_scissor_rect()
	{ return scissor_rect; }

	// Return GPU command queue
	inline ID3D12CommandQueue* DX12Graphics::get_command_queue()
	{ return command_queue; }

	// Return list of commands to submit to GPU
	inline ID3D12GraphicsCommandList* DX12Graphics::get_command_list()
	{ return command_list; }

	// Return memory used by the command list
	inline ID3D12CommandAllocator* DX12Graphics::get_command_list_alloc()
	{ return command_list_alloc; }

	// Return fence to synchronize CPU/GPU
	inline ID3D12Fence* DX12Graphics::get_fence()
	{ return fence; }
	
	// Return fence counter
	inline u64 DX12Graphics::get_current_fence()
	{ return current_fence; }
}

#endif  // PLATFORM_WINDOWS