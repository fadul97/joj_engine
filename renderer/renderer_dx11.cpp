#include "renderer_dx11.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "error.h"

JojRenderer::DX11Renderer::DX11Renderer()
{
    device = nullptr;
    context = nullptr;
    render_target_view = nullptr;
    swap_chain = nullptr;
}

JojRenderer::DX11Renderer::~DX11Renderer()
{
}

b8 JojRenderer::DX11Renderer::init(JojPlatform::Win32Window* window, JojGraphics::DX11Graphics* graphics)
{
    // Set window
    this->window = window;

    // Get objects created from graphics
    device = graphics->get_device();
    context = graphics->get_context();
    render_target_view = graphics->get_render_target_view();
    swap_chain = graphics->get_swap_chain();

    // --------------------------------
    // Input Assembler
    // --------------------------------

    // Setup vertex description
    D3D11_INPUT_ELEMENT_DESC input_layout[2] =
    {
        {
            "POSITION",                     // Semantic name
            0,                              // Semantic index
            DXGI_FORMAT_R32G32B32_FLOAT,    // Format of this vertex element (3D 32-bit float vector)
            0,                              // Input slot index this element will come from
            0,                              // Aligned byte offset
            D3D11_INPUT_PER_VERTEX_DATA,    // Input slot class (simple technique of instancing)
            0                               // Instance data step rate (0 = no advanced technique of instancing)
        },
        {
            "COLOR",                        // Semantic name
            0,                              // Semantic index
            DXGI_FORMAT_R32G32B32A32_FLOAT, // Format of this vertex element (3D 32-bit float vector)
            0,                              // Input slot index this element will come from
            12,                             // Aligned byte offset (4 bytes * XMFLOAT3 = 12)
            D3D11_INPUT_PER_VERTEX_DATA,    // Input slot class (simple technique of instancing)
            0                               // Instance data step rate (0 = no advanced technique of instancing)
        }
    };

    // --------------------------------
    // Vertex Buffer
    // --------------------------------

    // Triangle vertices
    JojRenderer::Vertex vertices[3] =
    {
        { DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Orange) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Purple) }
    };

    // Vertices syze in bytes
    const u32 vertices_size = 3 * sizeof(Vertex);

    // Setup vertex buffer description
    D3D11_BUFFER_DESC vb_desc;
    vb_desc.Usage = D3D11_USAGE_IMMUTABLE;          // Specify how the buffer will be used
    vb_desc.ByteWidth = vertices_size;              // Size, in bytes, of the vertex buffer to be created
    vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   // For a vertex buffer, specify D3D11_BIND_VERTEX_BUFFER
    vb_desc.CPUAccessFlags = 0;                     // Specify how the CPU will access the buffer
    vb_desc.MiscFlags = 0;                          // No miscellaneous needed
    vb_desc.StructureByteStride = 0;                // Size, in bytes, of a single element stored in the structured buffer

    // Create subresource - ?
    D3D11_SUBRESOURCE_DATA vinit_data;
    vinit_data.pSysMem = vertices;

    // Create vertex buffer
    ID3D11Buffer* vertex_buffer;
    ThrowIfFailed(device->CreateBuffer(
        &vb_desc,           // Description of buffer to create
        &vinit_data,        // Data to initialize buffer with
        &vertex_buffer));   // Return the created buffer

    // Bind vertex buffer to an input slot of device to pass vertices to pipeline as input
    u32 stride = sizeof(Vertex);    // size, in bytes, of an element in the corresponding vertex buffer
    u32 offset = 0;                 /* offset, in bytes, from the start of the vertex buffer to the position in the vertex buffer from which the
                                       input assembly should start reading the vertex data */

    context->IASetVertexBuffers(
        0,              // Input slot in which to start binding vertex buffers
        1,              // Number of buffers
        &vertex_buffer, // Pointer to the first element of an array of vertex buffers
        &stride,        // Pointer to the first element of an array of strides
        &offset);       // Pointer to the first element of an array of offsets.


    // --------------------------------
    // Index Buffer
    // --------------------------------

    // TODO:

    // --------------------------------
    // Vertex Shader
    // --------------------------------

    // TODO:

    // --------------------------------
    // Tessellation
    // --------------------------------

    // TODO:

    // --------------------------------
    // Geometry Shader + Stream Output
    // --------------------------------

    // TODO:

    // --------------------------------
    // Rasterizer
    // --------------------------------

    // TODO:

    // --------------------------------
    // Pixel Shader
    // --------------------------------

    // TODO:

    // --------------------------------
    // Output Merger
    // --------------------------------

    // TODO:
}

void JojRenderer::DX11Renderer::draw()
{

}

void JojRenderer::DX11Renderer::clear()
{

}