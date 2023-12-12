#include "my_game.h"

#include <d3dcompiler.h>
#include "error.h"
#include "engine.h"

void MyGame::init()
{
    JojEngine::Engine::renderer->reset_commands();

    // Build geometry and initialize pipeline
    build_geometry();
    build_root_signature();
    build_pipeline_state();

    JojEngine::Engine::renderer->submit_commands();
}

void MyGame::update()
{
	// Exit with ESCAPE key
	if (input->is_key_press(VK_ESCAPE))
		window->close();
}

void MyGame::display()
{
    JojEngine::Engine::renderer->clear(pipeline_state);

    // Submit pipeline configuration commands
    JojEngine::Engine::dx12_graphics->get_command_list()->SetGraphicsRootSignature(root_signature);
    vertex_buffer_view.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
    vertex_buffer_view.StrideInBytes = vertex_byte_stride;
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetVertexBuffers(0, 1, &vertex_buffer_view);
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Submit Drawing Commands
    JojEngine::Engine::dx12_graphics->get_command_list()->DrawInstanced(6, 1, 0, 0);

    JojEngine::Engine::renderer->present();
}

void MyGame::shutdown()
{
    root_signature->Release();
    pipeline_state->Release();
}

void MyGame::build_geometry()
{
    // --------------------------------
    // Vertex Buffer
    // --------------------------------

    // Triangle vertices
    JojRenderer::Vertex vertices[6] =
    {
        { DirectX::XMFLOAT3(-0.5f,  0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }
    };

    // Vertices size in bytes
    const u32 vertices_size = 6 * sizeof(JojRenderer::Vertex);

    // Set geometry attributes
    vertex_byte_stride = sizeof(JojRenderer::Vertex);
    vertex_buffer_size = vertices_size;

    // Allocate resources to the Vertex Buffer
    JojEngine::Engine::renderer->allocate_resource_in_cpu(vertices_size, &vertex_buffer_cpu);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::UPLOAD, vertices_size, &vertex_buffer_upload);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::GPU, vertices_size, &vertex_buffer_gpu);

    // Copy vertices to local 'mesh' storage
    JojEngine::Engine::renderer->copy_verts_to_cpu_blob(vertices, vertices_size, vertex_buffer_cpu);

    // Copy vertices to the GPU buffer using the Upload buffer
    JojEngine::Engine::renderer->copy_verts_to_gpu(vertices, vertices_size, vertex_buffer_upload, vertex_buffer_gpu);
}

void MyGame::build_root_signature()
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

void MyGame::build_pipeline_state()
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