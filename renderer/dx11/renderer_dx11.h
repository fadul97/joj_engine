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

		// Create index buffer
		ID3D11Buffer* create_index_buffer(u64 index_size, u32 index_count, const void* index_data);

		// Compile and create Vertex Shader from file
		ID3D11VertexShader* compile_and_create_vs_from_file(LPCWSTR file_path, ID3DBlob*& blob, unsigned long shader_flags);

		// Compile and create Pixel Shader from file
		ID3D11PixelShader* compile_and_create_ps_from_file(LPCWSTR file_path, ID3DBlob*& blob, unsigned long shader_flags);

		// Create and set input layout
		b8 create_and_set_input_layout(D3D11_INPUT_ELEMENT_DESC* input_desc, u32 array_size, ID3DBlob* blob, ID3D11InputLayout* input_layout);

		// Set primitive topology
		void set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY topology);

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

	// Set primitive topology
	inline void DX11Renderer::set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{ context->IASetPrimitiveTopology(topology); }
}

#endif  // PLATFORM_WINDOWS