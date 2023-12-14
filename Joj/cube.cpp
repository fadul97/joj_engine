#include "cube.h"

#include <d3dcompiler.h>
#include "error.h"
#include "engine.h"

void Cube::init()
{
    // Initialize members
    // Default members for handling pipeline
    root_signature = nullptr;
    pipeline_state = nullptr;

    // Buffers in CPU
    vertex_buffer_cpu = nullptr;
    index_buffer_cpu = nullptr;

    // Upload buffers: CPU -> GPU
    vertex_buffer_upload = nullptr;
    index_buffer_upload = nullptr;

    // Buffers in GPU
    vertex_buffer_gpu = nullptr;
    index_buffer_gpu = nullptr;

    // Buffer descriptors
    vertex_buffer_view = { 0 };	// Vertex buffer descriptor
    index_buffer_view = { 0 };		// Index buffer descriptor

    // Vertex buffer characteristics
    vertex_byte_stride = 0;
    vertex_buffer_size = 0;

    // Index buffer characteristics
    index_format = DXGI_FORMAT_UNKNOWN;
    index_buffer_size = 0;

    JojEngine::Engine::renderer->reset_commands();

    // Build geometry and initialize pipeline
    build_geometry();
    build_root_signature();
    build_pipeline_state();

    JojEngine::Engine::renderer->submit_commands();
}

void Cube::update()
{
	// Exit with ESCAPE key
	if (input->is_key_press(VK_ESCAPE))
		window->close();
}

void Cube::display()
{
    JojEngine::Engine::renderer->clear(pipeline_state);

    // Submit pipeline configuration commands
    JojEngine::Engine::dx12_graphics->get_command_list()->SetGraphicsRootSignature(root_signature);

    vertex_buffer_view.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
    vertex_buffer_view.StrideInBytes = vertex_byte_stride;
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetVertexBuffers(0, 1, &vertex_buffer_view);
    
    index_buffer_view.BufferLocation = index_buffer_gpu->GetGPUVirtualAddress();
    index_buffer_view.Format = index_format;
    index_buffer_view.SizeInBytes = index_buffer_size;
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetIndexBuffer(&index_buffer_view);

    JojEngine::Engine::dx12_graphics->get_command_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Submit Drawing Commands
    JojEngine::Engine::dx12_graphics->get_command_list()->DrawIndexedInstanced(36, 1, 0, 0, 0);

    JojEngine::Engine::renderer->present();
}

void Cube::shutdown()
{
    root_signature->Release();
    pipeline_state->Release();
}

void Cube::build_geometry()
{
    // --------------------------------
    // Vertex Buffer
    // --------------------------------

    // Geometry vertexes
    JojRenderer::Vertex vertices[8] =
    {
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }
    };

    // Geometry indexes
    u16 indices[36] =
    {
        // front face
        0, 1, 3,
        1, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    // ------------------------------------------------------------------
    // ------->> Transformation, Visualization and Projection <<---------
    // ------------------------------------------------------------------

    // World Matrix
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
    DirectX::XMMATRIX Ry = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(30));
    DirectX::XMMATRIX Rx = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(-30));
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(0, 0, 0);
    DirectX::XMMATRIX W = S * Ry * Rx * T;

    // View Matrix
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(0, 0, -6, 1);
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
    DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(pos, target, up);

    // Projection Matrix
    DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45),
        window->get_aspect_ratio(),
        1.0f, 100.0f);

    // Word-View-Projection Matrix
    DirectX::XMMATRIX WorldViewProj = W * V * P;

    // Place vertices in the projection window
    for (int i = 0; i < 8; ++i)
    {
        DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices[i].pos);
        DirectX::XMVECTOR proj = DirectX::XMVector3TransformCoord(vertex, WorldViewProj);
        DirectX::XMStoreFloat3(&vertices[i].pos, proj);
    }

    // -----------------------------------------------------------
    // >> Allocate and Copy Vertex and Index Buffers to the GPU <<
    // -----------------------------------------------------------

    // Byte size of vertices and indexes
    const u32 vb_size = 8 * sizeof(JojRenderer::Vertex);
    const u32 ib_size = 36 * sizeof(u16);

    // Setup geometry attributes
    vertex_byte_stride = sizeof(JojRenderer::Vertex);
    vertex_buffer_size = vb_size;
    index_format = DXGI_FORMAT_R16_UINT;
    index_buffer_size = ib_size;

    // Allocate resources to the Vertex Buffer
    JojEngine::Engine::renderer->allocate_resource_in_cpu(vb_size, &vertex_buffer_cpu);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::UPLOAD, vb_size, &vertex_buffer_upload);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::GPU, vb_size, &vertex_buffer_gpu);

    // Allocate resources to the Index Buffer
    JojEngine::Engine::renderer->allocate_resource_in_cpu(ib_size, &index_buffer_cpu);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::UPLOAD, ib_size, &index_buffer_upload);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::GPU, ib_size, &index_buffer_gpu);

    // Save a copy of the vertices and indexes in the 'mesh'
    JojEngine::Engine::renderer->copy_verts_to_cpu_blob(vertices, vb_size, vertex_buffer_cpu);
    JojEngine::Engine::renderer->copy_verts_to_cpu_blob(indices, ib_size, index_buffer_cpu);

    // Copy vertices and indexes to the GPU using the Upload buffer
    JojEngine::Engine::renderer->copy_verts_to_gpu(vertices, vb_size, vertex_buffer_upload, vertex_buffer_gpu);
    JojEngine::Engine::renderer->copy_verts_to_gpu(indices, ib_size, index_buffer_upload, index_buffer_gpu);
}

void Cube::build_root_signature()
{
    // TODO: comment specifications on root_sig_desc
    // Describe empty root signature
    D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
    root_sig_desc.NumParameters = 0;
    root_sig_desc.pParameters = nullptr;
    root_sig_desc.NumStaticSamplers = 0;
    root_sig_desc.pStaticSamplers = nullptr;
    root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // Serialize Root Signature
    ID3DBlob* serialized_root_sig = nullptr;
    ID3DBlob* error = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &root_sig_desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serialized_root_sig,
        &error));

    // Create empty root signature
    ThrowIfFailed(JojEngine::Engine::renderer->get_device()->CreateRootSignature(
        0,
        serialized_root_sig->GetBufferPointer(),
        serialized_root_sig->GetBufferSize(),
        IID_PPV_ARGS(&root_signature)));
}

void Cube::build_pipeline_state()
{
    // --------------------------------
    // Input Assembler
    // --------------------------------

    // Setup vertex description
    D3D12_INPUT_ELEMENT_DESC input_layout[2] =
    {
        {
            "POSITION",                                     // Semantic name
            0,                                              // Semantic index
            DXGI_FORMAT_R32G32B32_FLOAT,                    // Format of this vertex element (3D 32-bit float vector)
            0,                                              // Input slot index this element will come from
            0,                                              // Aligned byte offset
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     // Input slot class (simple technique of instancing)
            0                                               // Instance data step rate (0 = no advanced technique of instancing)
        },
        {
            "COLOR",                                        // Semantic name
            0,                                              // Semantic index
            DXGI_FORMAT_R32G32B32A32_FLOAT,                 // Format of this vertex element (3D 32-bit float vector)
            0,                                              // Input slot index this element will come from
            12,                                             // Aligned byte offset (4 bytes * XMFLOAT3 = 12)
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     // Input slot class (simple technique of instancing)
            0                                               // Instance data step rate (0 = no advanced technique of instancing)
        }
    };

    // --------------------
    // ----- Shaders ------
    // --------------------

    ID3DBlob* vertex_shader;
    ID3DBlob* pixel_shader;

    D3DReadFileToBlob(L"../vertex.cso", &vertex_shader);
    D3DReadFileToBlob(L"../pixel.cso", &pixel_shader);

    // --------------------
    // ---- Rasterizer ----
    // --------------------

    // TODO: comment specifications on rasterizer
    // Describe rasterizer
    D3D12_RASTERIZER_DESC rasterizer = {};
    rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    //rasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
    rasterizer.CullMode = D3D12_CULL_MODE_BACK;
    //rasterizer.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer.FrontCounterClockwise = FALSE;
    rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer.DepthClipEnable = TRUE;
    rasterizer.MultisampleEnable = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;
    rasterizer.ForcedSampleCount = 0;
    rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ---------------------
    // --- Color Blender ---
    // ---------------------

    // TODO: comment specifications on blender
    // Describe blender
    D3D12_BLEND_DESC blender = {};
    blender.AlphaToCoverageEnable = FALSE;
    blender.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    for (u32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blender.RenderTarget[i] = default_render_target_blend_desc;

    // ---------------------
    // --- Depth Stencil ---
    // ---------------------

    // TODO: comment specifications on depth_stencil
    // Describe Depth/Stencil
    D3D12_DEPTH_STENCIL_DESC depth_stencil = {};
    depth_stencil.DepthEnable = TRUE;
    depth_stencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depth_stencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depth_stencil.StencilEnable = FALSE;
    depth_stencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depth_stencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC default_stencil_op =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    depth_stencil.FrontFace = default_stencil_op;
    depth_stencil.BackFace = default_stencil_op;

    // -----------------------------------
    // --- Pipeline State Object (PSO) ---
    // -----------------------------------

    // TODO: comment specifications on pso
    // Describe Depth/Stencil
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = root_signature;
    pso.VS = { reinterpret_cast<BYTE*>(vertex_shader->GetBufferPointer()), vertex_shader->GetBufferSize() };
    pso.PS = { reinterpret_cast<BYTE*>(pixel_shader->GetBufferPointer()), pixel_shader->GetBufferSize() };
    pso.BlendState = blender;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = rasterizer;
    pso.DepthStencilState = depth_stencil;
    pso.InputLayout = { input_layout, 2 };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso.SampleDesc.Count = JojEngine::Engine::dx12_graphics->get_antialiasing();
    pso.SampleDesc.Quality = JojEngine::Engine::dx12_graphics->get_quality();
    JojEngine::Engine::dx12_graphics->get_device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline_state));

    vertex_shader->Release();
    pixel_shader->Release();
}