#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "renderer.h"
#include "dx11/context_dx11.h"
#include <d3d11.h>      // Main Direct3D functions

namespace JojRenderer
{
	class DX11Renderer : public Renderer
	{
	public:
		DX11Renderer();
		~DX11Renderer();

		b8 init(std::unique_ptr<JojPlatform::Window>& window);		// Initialize renderer
		void render();			// Draw to screen
		void clear();			// Clear screen
		void swap_buffers();	// Change front and back buffers
		void shutdown();	// Clear resources

		//std::unique_ptr<ID3D11Device>& get_device();					// Pass refence of device but keep ownership after function
		//std::unique_ptr<ID3D11DeviceContext>& get_device_context();	// Pass refence of device context but keep ownership after function

		ID3D11Device* get_device();					// Return Graphics device
		ID3D11DeviceContext* get_device_context();	// Return Graphics device context

		// ---------------------------------------------------
		// Handle resources
		// ---------------------------------------------------

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

		// ---------------------------------------------------
		// Change pipeline state
		// ---------------------------------------------------

		// Set primitive topology
		void set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY topology);

	private:
		//std::unique_ptr<JojGraphics::DX11Context> context;
		JojGraphics::DX11Context* context;


		ID3D11Device* device;					// Graphics device
		ID3D11DeviceContext* device_context;	// Graphics device context

		f32 bg_color[4];	// Backbuffer background color

		// ---------------------------------------------------
		// Pipeline members
		// ---------------------------------------------------
		u32 antialiasing;								// Number of samples for each pixel on the screen
		u32 quality;									// Antialiasing sampling quality
		b8 vsync;										// Vertical sync 
		IDXGISwapChain* swap_chain;						// Swap chain
		ID3D11RenderTargetView* render_target_view;     // Backbuffer render target view
		ID3D11DepthStencilView* depth_stencil_view;		// Depth/Stencil view
		D3D11_VIEWPORT viewport;						// Viewport
		ID3D11BlendState* blend_state;					// Color mix settings
		ID3D11RasterizerState* rasterizer_state;	// Rasterizer state
	};

	// Return Graphics device
	inline ID3D11Device* DX11Renderer::get_device()
	{ return device; }

	// Return Graphics device context
	inline ID3D11DeviceContext* DX11Renderer::get_device_context()
	{ return device_context; }

/*
	// Pass refence of device but keep ownership after function
	inline std::unique_ptr<ID3D11Device>& DX11Renderer::get_device()
	{ return device; }

	// Pass refence of device context but keep ownership after function
	inline std::unique_ptr<ID3D11DeviceContext>& DX11Renderer::get_device_context()
	{ return device_context; }
*/

	// Set primitive topology
	inline void DX11Renderer::set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{ device_context->IASetPrimitiveTopology(topology); }
}

#endif // PLATFORM_WINDOWS