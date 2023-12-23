#include "graphics_dx11.h"

#if PLATFORM_WINDOWS

#include <sstream>
#include "error.h"
#include <d3d11sdklayers.h>

JojGraphics::DX11Graphics::DX11Graphics()
{
	// Configuration
	antialiasing = 1;		// No antialising
	quality = 0;			// Default quality
	vsync = false;			// No vertical sync

	device = nullptr;
	factory = nullptr;
	context = nullptr;
	viewport = { 0 };

	swap_chain = nullptr;
	render_target_view = nullptr;
	depth_stencil_view = nullptr;
	blend_state = nullptr;
	feature_level = D3D_FEATURE_LEVEL_11_0;

	bg_color[0] = 0.0f;                       // Red
	bg_color[1] = 0.0f;                       // Green
	bg_color[2] = 0.0f;                       // Blue
	bg_color[3] = 0.0f;                       // Alpha (0 = transparent)
}

JojGraphics::DX11Graphics::~DX11Graphics()
{
	// Release blend state
	if (blend_state)
		blend_state->Release();

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
	if (context)
	{
		// Restore to original state
		context->ClearState();
		context->Release();
	}

	// Release graphics device
	if (device)
		device->Release();
}

void JojGraphics::DX11Graphics::log_hardware_info()
{
	const u32 bytes_in_megabyte = 1048576U;

	// --------------------------------------
	// Video adapter (Graphics card)
	// --------------------------------------
	IDXGIAdapter* adapter = nullptr;
	if (factory->EnumAdapters(0, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstringstream text;
		text << L"---> Graphics card: " << desc.Description << L"\n";
		OutputDebugStringW(text.str().c_str());
	}

	IDXGIAdapter4* adapter4 = nullptr;
	if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter4))))
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO mem_info;
		adapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &mem_info);

		std::wstringstream text;
		text << L"---> Video memory (free): " << mem_info.Budget / bytes_in_megabyte << L"MB\n";
		text << L"---> Video memory (used): " << mem_info.CurrentUsage / bytes_in_megabyte << L"MB\n";
		OutputDebugStringW(text.str().c_str());

		adapter4->Release();
	}

	// -----------------------------------------
	// TODO: Max feature Level supported by GPU
	// -----------------------------------------
	
	// Instructions block
	{
		std::wstringstream text;
		text << L"---> Feature Level: 11_0\n";
		OutputDebugStringW(text.str().c_str()); 
	}

	// -----------------------------------------
	// Video output (monitor)
	// -----------------------------------------

	IDXGIOutput* output = nullptr;
	if (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstringstream text;
		text << L"---> Monitor: " << desc.DeviceName << L"\n";
		OutputDebugStringW(text.str().c_str());
	}

	// ------------------------------------------
	// Video mode (resolution)
	// ------------------------------------------

	// Get screen dimensions
	u32 dpi = GetDpiForSystem();
	u32 screen_width = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
	u32 screen_height = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);

	// Get screen update frequency
	DEVMODE dev_mode = { 0 };
	dev_mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dev_mode);
	u32 refresh = dev_mode.dmDisplayFrequency;

	std::wstringstream text;
	text << L"---> Resolution: " << screen_width << L"x" << screen_height << L" " << refresh << L" Hz\n";
	OutputDebugStringW(text.str().c_str());

	// ------------------------------------------

	// Release used DXGI interfaces
	if (adapter) adapter->Release();
	if (output) output->Release();
}

void JojGraphics::DX11Graphics::init(JojPlatform::Window* window)
{
	// ---------------------------------------------------
	// Creates DXGI infrastructure and D3D device
	// ---------------------------------------------------

	u32 factory_flags = 0;
	u32 create_device_flags = 0;

#ifndef _DEBUG
	// Enable DXGI debug layer
	factory_flags = DXGI_CREATE_FACTORY_DEBUG;

	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;

	// Enable D3D11 debug layer
	ID3D11Debug* debug_controller;
	ThrowIfFailed(D3D11GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
	debug_controller->EnableDebugLayer();
#endif // !_DEBUG

	// Create object for DirectX graphics infrastructure (DXGI)
	ThrowIfFailed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

	if FAILED(D3D11CreateDevice(
		NULL,						// Default adapter
		D3D_DRIVER_TYPE_HARDWARE,	// D3D driver type (Hardware, Reference or Software)
		NULL,						// Pointer to software rasterizer - No software device
		create_device_flags,		// Debug or Normal Mode
		NULL,						// Default feature level (NULL = max supported)
		0,							// Size of feature level array
		D3D11_SDK_VERSION,			// Direct3D SDK version
		&device,					// Stores D3D device created
		&feature_level,				// Current Direct3D version in use,
		&context))					// D3D context device
	{
		if FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP,
			NULL, create_device_flags, NULL, 0, D3D11_SDK_VERSION,
			&device, &feature_level, &context))
		{
			OutputDebugString("--> Failed to create device, using WARP Adapter\n");
		}
	}

	// Get pointer to Direct3D device 
	IDXGIDevice* dxgi_device = nullptr;
	ThrowIfFailed(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device));

	// Get Adpapter from Direct3D device (d3d11Device)
	IDXGIAdapter* dxgi_adapter = nullptr;
	ThrowIfFailed(dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgi_adapter));
	

	// Get pointer to adapter DXGIFactory
	IDXGIFactory* dxgi_factory = nullptr;
	ThrowIfFailed(dxgi_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgi_factory));

	// Display graphics hardware information in Visual Studio Output
#ifdef _DEBUG
	log_hardware_info();
#endif 

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
	//swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// Default value for Flags
	//swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// Default mode for scaling
	swap_chain_desc.SampleDesc.Count = antialiasing;										// Samples per pixel (antialiasing)
	swap_chain_desc.SampleDesc.Quality = quality;											// Level of image quality
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;							// Use surface as Render Target
	swap_chain_desc.BufferCount = 2;														// Number of buffers (Front + Back)
	swap_chain_desc.OutputWindow = window->get_id();										// Window ID
	swap_chain_desc.Windowed = (window->get_mode() == JojPlatform::WindowMode::WINDOWED);	// Fullscreen or windowed 
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;									// Discard surface after presenting
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;							// Use Back buffer size for Fullscreen

	// Create Swap Chain
	ThrowIfFailed(dxgi_factory->CreateSwapChain(device, &swap_chain_desc, &swap_chain));

	// ---------------------------------------------------
	// Render Target View
	// ---------------------------------------------------
	
	// Get backbuffer surface of a Swap Chain
	ID3D11Texture2D* backbuffer = nullptr;
	ThrowIfFailed(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer)));
	
	// Create render target view for backbuffer
	ThrowIfFailed(device->CreateRenderTargetView(backbuffer, NULL, &render_target_view));

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
	ThrowIfFailed(device->CreateTexture2D(&depth_stencil_desc, 0, &depth_stencil_buffer));

	// Create Depth/Stencil View
	ThrowIfFailed(device->CreateDepthStencilView(depth_stencil_buffer, 0, &depth_stencil_view));

	// Bind render target and depth stencil to the Output Merger stage
	context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

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
	context->RSSetViewports(1, &viewport);

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
	ThrowIfFailed(device->CreateBlendState(&blend_desc, &blend_state));

	// Bind blend state to the Output Merger stage
	context->OMSetBlendState(blend_state, nullptr, 0xffffffff);

	// ---------------------------------------------------
	//	Release DXGI Interfaces
	// ---------------------------------------------------

	dxgi_device->Release();
	dxgi_adapter->Release();
	dxgi_factory->Release();
	backbuffer->Release();
}

#endif	// PLATFORM_WINDOWS