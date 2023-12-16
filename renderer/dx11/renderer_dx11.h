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
		void swap_buffers();														// Swap buffers

		// Create vertex buffer
		// TODO: Check if using templates is better than using void*
		ID3D11Buffer* create_vertex_buffer(u64 vertex_size, u32 vertex_count, const void* vertex_data);
		ID3D11Buffer* create_index_buffer(u64 index_size, u32 index_count, const void* index_data);

	private:
		JojPlatform::Window* window;				// Engine window
		JojGraphics::DX11Graphics* graphics;		// Graphics context

		// Settings
		f32 bg_color[4];
		b8 vsync;

		ID3D11Device* device;						// Graphics device
		ID3D11DeviceContext* context;				// Graphics device context
		ID3D11RenderTargetView* render_target_view; // Backbuffer render target view
		ID3D11DepthStencilView* depth_stencil_view; // Depth/Stencil view
		IDXGISwapChain* swap_chain;					// Swap chain
		ID3D11RasterizerState* rasterizer_state;	// Rasterizer state
	};
}

#endif  // PLATFORM_WINDOWS