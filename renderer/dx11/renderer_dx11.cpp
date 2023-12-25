#include "renderer_dx11.h"

#include <dxgi.h>
#include <d3dcompiler.h>

JojRenderer::DX11Renderer::DX11Renderer()
{
	context = nullptr;

	device = nullptr;
	device_context = nullptr;

	// ---------------------------------------------------
	// Pipeline members
	// ---------------------------------------------------
	antialiasing = 1;				// No antialising
	quality = 0;					// Default quality
	vsync = false;					// No vertical sync
	swap_chain = nullptr;			// Swap chain
	render_target_view = nullptr;   // Backbuffer render target view
	depth_stencil_view = nullptr;	// Depth/Stencil view
	viewport = { 0 };				// Viewport
	blend_state = nullptr;			// Color mix settings
	rasterizer_state = nullptr;		// Rasterizer state

	// Background color
	bg_color[0] = 0.0f;		// Red
	bg_color[1] = 0.0f;		// Green
	bg_color[2] = 0.0f;		// Blue
	bg_color[3] = 0.0f;		// Alpha (0 = transparent, 1 = solid)
}

JojRenderer::DX11Renderer::~DX11Renderer()
{
	// Release rasterizer state
	if (rasterizer_state)
		rasterizer_state->Release();

	// Release blend state
	if (blend_state)
		blend_state->Release();

	// Release depth stencil view
	if (depth_stencil_view)
		depth_stencil_view->Release();

	// Release render target view
	if (render_target_view)
		render_target_view->Release();

	// Release swap chain
	if (swap_chain)
	{
		// Direct3D is unable to close when full screen
		swap_chain->SetFullscreenState(false, NULL);
		swap_chain->Release();
	}

	//  Release graphics device context
	if (device_context)
	{
		// Restore to original state
		device_context->ClearState();
		device_context->Release();
		device_context = nullptr;
	}

	context->debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	OutputDebugString("\n\n\n");

	// Release graphics device
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	// Release Dx11 Context
	if (context)
		delete context;
}

b8 JojRenderer::DX11Renderer::init(std::unique_ptr<JojPlatform::Window>& window)
{
	// Background color of the backbuffer = window background color
	COLORREF color = window->get_color();

	bg_color[0] = GetRValue(color) / 255.0f;	// Red
	bg_color[1] = GetGValue(color) / 255.0f;	// Green
	bg_color[2] = GetBValue(color) / 255.0f;	// Blue
	bg_color[3] = 1.0f;							// Alpha (1 = solid)

	//context = std::make_unique<JojGraphics::DX11Context>();
	context = new JojGraphics::DX11Context();

	context->init(window);

	// Get device and device context ownership
	//device.reset(context->get_device());
	//device_context.reset(context->get_context());

	device = context->get_device();
	device_context = context->get_context();

	// ------------------------------------------------------------------------------------------------------
	//                                          PIPELINE SETUP
	// ------------------------------------------------------------------------------------------------------

	// ---------------------------------------------------
	// Swap Chain
	// ---------------------------------------------------

	// Describe Swap Chain
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 };
	swap_chain_desc.BufferDesc.Width = u32(window->get_width());							// Back buffer width
	swap_chain_desc.BufferDesc.Height = u32(window->get_height());							// Back buffer height
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;									// Refresh rate in hertz 
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 1;									// Numerator is an int
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;							// Color format - RGBA 8 bits
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;		// Default value for Flags
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;						// Default mode for scaling
	swap_chain_desc.SampleDesc.Count = antialiasing;										// Samples per pixel (antialiasing)
	swap_chain_desc.SampleDesc.Quality = quality;											// Level of image quality
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;							// Use surface as Render Target
	swap_chain_desc.BufferCount = 2;														// Number of buffers (Front + Back)
	swap_chain_desc.OutputWindow = window->get_id();										// Window ID
	swap_chain_desc.Windowed = (window->get_mode() == JojPlatform::WindowMode::WINDOWED);	// Fullscreen or windowed 
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;								// Discard surface after presenting
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;							// Use Back buffer size for Fullscreen

	// Create Swap Chain
	if FAILED(context->get_factory()->CreateSwapChain(device, &swap_chain_desc, &swap_chain))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to CreateSwapChain.\n");
		return false;
	}

	// ---------------------------------------------------
	// Render Target View
	// ---------------------------------------------------

	// Get backbuffer surface of a Swap Chain
	ID3D11Texture2D* backbuffer = nullptr;
	if FAILED(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer)))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to Get backbuffer surface of a Swap Chain.\n");
		return false;
	}

	// Create render target view for backbuffer
	if FAILED(device->CreateRenderTargetView(backbuffer, NULL, &render_target_view))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to CreateRenderTargetView.\n");
		return false;
	}

	// ---------------------------------------------------
	// Depth/Stencil View
	// ---------------------------------------------------

	// Describe Depth/Stencil Buffer Desc
	D3D11_TEXTURE2D_DESC depth_stencil_desc = { 0 };
	depth_stencil_desc.Width = u32(window->get_width());		// Depth/Stencil buffer width
	depth_stencil_desc.Height = u32(window->get_height());		// Depth/Stencil buffer height
	depth_stencil_desc.MipLevels = 0;							// Number of mipmap levels
	depth_stencil_desc.ArraySize = 1;							// Number of textures in array
	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// Color format - Does it need to be the same format of swapChainDesc?
	depth_stencil_desc.SampleDesc.Count = antialiasing;			// Samples per pixel (antialiasing)
	depth_stencil_desc.SampleDesc.Quality = quality;			// Level of image quality
	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;				// Default - GPU will both read and write to the resource
	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// Where resource will be bound to the pipeline
	depth_stencil_desc.CPUAccessFlags = 0;						// CPU will not read not write to the Depth/Stencil buffer
	depth_stencil_desc.MiscFlags = 0;							// Optional flags

	// Create Depth/Stencil Buffer
	ID3D11Texture2D* depth_stencil_buffer;
	if FAILED(device->CreateTexture2D(&depth_stencil_desc, 0, &depth_stencil_buffer))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to CreateTexture2D.\n");
		return false;
	}

	// Create Depth/Stencil View
	if FAILED(device->CreateDepthStencilView(depth_stencil_buffer, 0, &depth_stencil_view))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to CreateDepthStencilView.\n");
		return false;
	}

	// Bind render target and depth stencil to the Output Merger stage
	device_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

	// ---------------------------------------------------
	// Viewport
	// ---------------------------------------------------

	// Describe Viewport
	viewport.TopLeftY = 0.0f;
	viewport.TopLeftX = 0.0f;
	viewport.Width = static_cast<f32>(window->get_width());
	viewport.Height = static_cast<f32>(window->get_height());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Set Viewport
	device_context->RSSetViewports(1, &viewport);

	// ---------------------------------------------
	// Blend State
	// ---------------------------------------------

	// Describe blend state
	D3D11_BLEND_DESC blend_desc = { };
	blend_desc.AlphaToCoverageEnable = false;                                // Highlight the silhouette of sprites
	blend_desc.IndependentBlendEnable = false;                               // Use the same mix for all render targets
	blend_desc.RenderTarget[0].BlendEnable = true;                           // Enable blending
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;             // Source mixing factor
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;        // Target of RGB mixing is inverted alpha
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;                 // Addition operation in color mixing
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;        // Alpha blend source is the alpha of the pixel shader
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;   // Fate of Alpha mixture is inverted alpha
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;            // Addition operation in color mixing
	blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0F;                 // Components of each pixel that can be overwritten

	// Create blend state
	if FAILED(device->CreateBlendState(&blend_desc, &blend_state))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to CreateBlendState.\n");
		return false;
	}

	// Bind blend state to the Output Merger stage
	device_context->OMSetBlendState(blend_state, nullptr, 0xffffffff);

	// ---------------------------------------------------
	// Rasterizer
	// ---------------------------------------------------

	// TODO: comment specifications on rasterizer
	// Describe rasterizer
	D3D11_RASTERIZER_DESC rasterizer_desc = {};
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
	//rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	//rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.DepthClipEnable = true;

	// Create rasterizer state
	device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

	// Set rasterizer state
	device_context->RSSetState(rasterizer_state);

	// ---------------------------------------------------
	//	Release Resources
	// ---------------------------------------------------

	backbuffer->Release();
	depth_stencil_buffer->Release();

	return true;
}

void JojRenderer::DX11Renderer::render()
{
	// TODO:
}

void JojRenderer::DX11Renderer::clear()
{
	device_context->ClearRenderTargetView(render_target_view, bg_color);
	device_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void JojRenderer::DX11Renderer::swap_buffers()
{
	swap_chain->Present(vsync, NULL);
	device_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
}

void JojRenderer::DX11Renderer::shutdown()
{
}

ID3D11Buffer* JojRenderer::DX11Renderer::create_vertex_buffer(u64 vertex_size, u32 vertex_count, const void* vertex_data)
{
	// TODO: comment specifications on buffer_desc
	// Describe Buffer - Resource structure
	D3D11_BUFFER_DESC buffer_desc = { 0 };
	buffer_desc.ByteWidth = vertex_size * vertex_count;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	// Set data we want to initialize the buffer contents with
	D3D11_SUBRESOURCE_DATA srd = { vertex_data, 0, 0 };

	// Create Buffer
	ID3D11Buffer* vertex_buffer = nullptr;
	if FAILED(device->CreateBuffer(&buffer_desc, &srd, &vertex_buffer))
		MessageBoxA(nullptr, "Failed to create Vertex Buffer", 0, 0);

	return vertex_buffer;
}

ID3D11Buffer* JojRenderer::DX11Renderer::create_index_buffer(u64 index_size, u32 index_count, const void* index_data)
{
	// Describe index buffer
	D3D11_BUFFER_DESC index_buffer_desc;
	index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	index_buffer_desc.ByteWidth = index_size * index_count;
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	// Specify the data to initialize the index buffer.
	D3D11_SUBRESOURCE_DATA indices_init_data;
	indices_init_data.pSysMem = index_data;

	// Create the index buffer.
	ID3D11Buffer* index_buffer = nullptr;
	if FAILED(device->CreateBuffer(&index_buffer_desc, &indices_init_data, &index_buffer))
		MessageBoxA(nullptr, "Failed to create Index Buffer", 0, 0);

	return index_buffer;
}

ID3D11VertexShader* JojRenderer::DX11Renderer::compile_and_create_vs_from_file(LPCWSTR file_path, ID3DBlob*& blob, unsigned long shader_flags)
{
	ID3DBlob* compile_errors_blob;  // To get info about compilation

	if FAILED(D3DCompileFromFile(file_path, nullptr, nullptr, "main", "vs_5_0", shader_flags, NULL, &blob, &compile_errors_blob))
		MessageBoxA(nullptr, "Failed to compile Vertex Shader.", 0, 0);

	ID3D11VertexShader* vertex_shader = nullptr;
	if FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertex_shader))
		MessageBoxA(nullptr, "Failed to create Vertex Shader.", 0, 0);

	return vertex_shader;
}

ID3D11PixelShader* JojRenderer::DX11Renderer::compile_and_create_ps_from_file(LPCWSTR file_path, ID3DBlob*& blob, unsigned long shader_flags)
{
	ID3DBlob* compile_errors_blob;  // To get info about compilation

	if FAILED(D3DCompileFromFile(file_path, nullptr, nullptr, "main", "ps_5_0", shader_flags, NULL, &blob, &compile_errors_blob))
		MessageBoxA(nullptr, "Failed to compile Pixel Shader.", 0, 0);

	ID3D11PixelShader* pixel_shader = nullptr;
	if FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixel_shader))
		MessageBoxA(nullptr, "Failed to create Vertex Shader.", 0, 0);

	return pixel_shader;
}

b8 JojRenderer::DX11Renderer::create_and_set_input_layout(D3D11_INPUT_ELEMENT_DESC* input_desc, u32 array_size, ID3DBlob* blob, ID3D11InputLayout* input_layout)
{
	// Create input layout
	if FAILED(device->CreateInputLayout(input_desc, array_size, blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout))
	{
		MessageBoxA(nullptr, "Failed to create Input Layout.", 0, 0);
		return false;
	}

	// Bind input layout to the Input Assembler Stage
	device_context->IASetInputLayout(input_layout);

	return true;
}