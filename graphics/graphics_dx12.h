#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <dxgi1_6.h>    // DirectX graphics infrastructure
#include <d3d12.h>      // Main Direct3D functions
#include "window_win32.h"

namespace JojGraphics
{
	class Dx12Graphics
	{
	public:
		Dx12Graphics();
		~Dx12Graphics();

		void init(JojPlatform::Win32Window* window);	// Initialize Direct3D

	private:
		// Pipeline
		ID3D12Device4* device;							// Graphics device
		IDXGIFactory6* factory;							// Main DXGI interface

		ID3D12CommandQueue* command_queue;              // GPU command queue
		ID3D12GraphicsCommandList* command_list;        // List of commands to submit to GPU
		ID3D12CommandAllocator* command_list_alloc;     // Memory used by the command list

		// Synchronization                         
		ID3D12Fence* fence;			// Fence to synchronize CPU/GPU
		u64 current_fence;			// Fence counter

		void log_hardware_info();	// Show hardware information
		b8 wait_command_queue();	// Wait for command queue execution
		void submit_commands();		// Submit pending commands for execution
	};
}

#endif  // PLATFORM_WINDOWS