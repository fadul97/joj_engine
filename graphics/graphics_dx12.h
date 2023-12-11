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

		ID3D12Device4* get_device();					// Return graphics device
		IDXGIFactory6* get_factory();					// Return DXGI interface


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

		void log_hardware_info();	// Show hardware information
	};

	// Return graphics device
	inline ID3D12Device4* DX12Graphics::get_device()
	{ return device; }

	// Return DXGI interface
	inline IDXGIFactory6* DX12Graphics::get_factory()
	{ return factory; }
}

#endif  // PLATFORM_WINDOWS