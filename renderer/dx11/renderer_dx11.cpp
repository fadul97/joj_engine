#include "renderer_dx11.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "error.h"

JojRenderer::DX11Renderer::DX11Renderer()
{
    window = nullptr;
    graphics = nullptr;

    device = nullptr;
    context = nullptr;
    render_target_view = nullptr;
    depth_stencil_view = nullptr;
    swap_chain = nullptr;
    rasterizer_state = nullptr;

    // Background color
    bg_color[0] = 0.0f;		// Red
    bg_color[1] = 0.0f;		// Green
    bg_color[2] = 0.0f;		// Blue
    bg_color[3] = 0.0f;		// Alpha (0 = transparent, 1 = solid)

    vsync = false;
}

JojRenderer::DX11Renderer::~DX11Renderer()
{
}

b8 JojRenderer::DX11Renderer::init(JojPlatform::Window* window, JojGraphics::DX11Graphics* graphics)
{
    // Set window
    this->window = window;

    // Background color of the backbuffer = window background color
    COLORREF color = window->get_color();

    bg_color[0] = GetRValue(color) / 255.0f;	// Red
    bg_color[1] = GetGValue(color) / 255.0f;	// Green
    bg_color[2] = GetBValue(color) / 255.0f;	// Blue
    bg_color[3] = 1.0f;							// Alpha (1 = solid)

    // Get objects created from graphics
    device = graphics->get_device();
    context = graphics->get_context();
    render_target_view = graphics->get_render_target_view();
    depth_stencil_view = graphics->get_depth_stencil_view();
    swap_chain = graphics->get_swap_chain();

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
    context->RSSetState(rasterizer_state);

    return true;
}

void JojRenderer::DX11Renderer::draw()
{

}

void JojRenderer::DX11Renderer::clear()
{
    context->ClearRenderTargetView(render_target_view, bg_color);
    context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void JojRenderer::DX11Renderer::swap_buffers()
{
    swap_chain->Present(vsync, NULL);
    context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
}