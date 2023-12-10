#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <dxgi1_6.h>	// DirectX graphics infrastructure
#include <d3d11.h>      // Main Direct3D functions
#include "window_win32.h"

namespace JojGraphics
{
	class DX11Graphics
	{
	public:
		DX11Graphics();
		~DX11Graphics();

		void init(JojPlatform::Win32Window* window);	// Initialize Direct3D
		void set_vsync(b8 state);							// Vertical sync on/off
		void clear();                                   // Clear backbuffer with background color
		void present();                                 // Present drawing on screen

	private:
		ID3D11Device* device;							// Graphics device
		IDXGIFactory6* factory;							// Main DXGI interface
		ID3D11DeviceContext* context;                   // Graphics device context
		D3D11_VIEWPORT viewport;						// Viewport

		IDXGISwapChain* swap_chain;						// Swap chain             
		ID3D11RenderTargetView* render_target_view;     // Backbuffer render target view
		ID3D11BlendState* blend_state;					// Color mix settings
		D3D_FEATURE_LEVEL feature_level;				// Level of D3D features supported by hardware
		f32 bg_color[4];								// Backbuffer background color
		b8 vsync;										// Vertical sync 

		void log_hardware_info();	// Show hardware information
		b8 wait_command_queue();	// Wait for command queue execution
		void submit_commands();		// Submit pending commands for execution
	};

	// Vertical sync on/off
	inline void DX11Graphics::set_vsync(b8 state)
	{ vsync = state; }
}

#endif	// PLATFORM_WINDOWS