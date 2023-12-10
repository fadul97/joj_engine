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
		void set_vsync(b8 state);						// Vertical sync on/off
		void clear();                                   // Clear backbuffer with background color
		void present();                                 // Present drawing on screen

		ID3D11Device* get_device();						// Return graphics device
		ID3D11DeviceContext* get_context();				// Return graphics device context
		D3D11_VIEWPORT get_viewport();					// Return viewport

		IDXGISwapChain* get_swap_chain();					// Return swap chain    
		ID3D11RenderTargetView* get_render_target_view();	// Return backbuffer render target view

	private:
		ID3D11Device* device;							// Graphics device
		ID3D11DeviceContext* context;                   // Graphics device context
		D3D11_VIEWPORT viewport;						// Viewport

		IDXGIFactory6* factory;							// Main DXGI interface

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

	// Return graphics device
	inline ID3D11Device* DX11Graphics::get_device()
	{ return device; }

	// Return graphics device context
	inline ID3D11DeviceContext* DX11Graphics::get_context()
	{ return context; }

	// Return viewport
	inline D3D11_VIEWPORT DX11Graphics::get_viewport()
	{ return viewport; }

	// Return swap chain 
	inline IDXGISwapChain* DX11Graphics::get_swap_chain()
	{ return swap_chain; }

	// Return backbuffer render target view
	inline ID3D11RenderTargetView* DX11Graphics::get_render_target_view()
	{ return render_target_view; }

	// Vertical sync on/off
	inline void DX11Graphics::set_vsync(b8 state)
	{ vsync = state; }
}

#endif	// PLATFORM_WINDOWS