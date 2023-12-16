#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "window.h"
#include "dx11/graphics_dx11.h"
#include <DirectXColors.h>

namespace JojRenderer
{
	class DX11Renderer
	{
	public:
		DX11Renderer();
		~DX11Renderer();

		b8 init(JojPlatform::Window* window, JojGraphics::DX11Graphics* graphics);	// Initialize renderer
		void draw();																// Draw
		void clear();																// Clear backbuffer for next frame
		void swap_buffers();

	private:
		JojPlatform::Window* window;
		f32 bg_color[4];
		b8 vsync;

		ID3D11Device* device;						// Graphics device
		ID3D11DeviceContext* context;				// Graphics device context
		ID3D11RenderTargetView* render_target_view; // Backbuffer render target view
		ID3D11DepthStencilView* depth_stencil_view; // Depth/Stencil view
		IDXGISwapChain* swap_chain;					// Swap chain
	};
}

#endif  // PLATFORM_WINDOWS