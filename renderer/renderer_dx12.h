#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "window_win32.h"
#include "graphics_dx12.h"
#include <DirectXColors.h>

namespace JojRenderer
{
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};

	class DX12Renderer
	{
	public:
		DX12Renderer();

		/* ATTENTION: Call DX12Renderer destructor before DX12Graphics destructor */
		~DX12Renderer();

		b8 init(JojPlatform::Win32Window* window, JojGraphics::DX12Graphics* graphics);	// Initialize renderer
		void draw();																	// Draw
		void clear();																	// Clear backbuffer for next frame

	private:
		JojPlatform::Win32Window* window;

		// Pipeline
		ID3D12Device* device;							// Graphics device
		IDXGIFactory6* factory;							// Main DXGI interface
		IDXGISwapChain* swap_chain;						// Swap chain

		ID3D12CommandQueue* command_queue;              // GPU command queue
		ID3D12GraphicsCommandList* command_list;        // List of commands to submit to GPU
		ID3D12CommandAllocator* command_list_alloc;     // Memory used by the command list

		// Synchronization                         
		ID3D12Fence* fence;								// Fence to synchronize CPU/GPU
		u64 current_fence;								// Fence counter

		b8 wait_command_queue();	// Wait for command queue execution
		void submit_commands();		// Submit pending commands for execution
	};
}

#endif  // PLATFORM_WINDOWS