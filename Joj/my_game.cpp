#include "my_game.h"

#include <d3dcompiler.h>
#include "renderer_dx12.h"
#include "graphics_dx12.h"
#include "error.h"
#include "engine.h"

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

u32 vertexByteStride = 0;
u32 vertexBufferSize = 0;

// buffers na CPU
ID3DBlob* vertex_buffer_cpu;

// buffers de Upload CPU -> GPU
ID3D12Resource* vertexBufferUpload;

// buffers na GPU
ID3D12Resource* vertexBufferGPU;

ID3D12RootSignature* rootSignature;
ID3D12PipelineState* pipelineState;

// descritor do vertex buffer
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

void MyGame::init()
{
    // Initialize pipeline
    JojEngine::Engine::renderer->reset_commands();

    // --------------------------------
    // Vertex Buffer
    // --------------------------------

    // Triangle vertices
    JojRenderer::Vertex vertices[6] =
    {
        { DirectX::XMFLOAT3(-0.5f,  0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue) },
        { DirectX::XMFLOAT3( 0.5f,  0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue) }
    };

    // Vertices syze in bytes
    const u32 vbSize = 6 * sizeof(JojRenderer::Vertex);

    vertexByteStride = sizeof(JojRenderer::Vertex);
    vertexBufferSize = vbSize;

    // Allocate (2 parameters)
    //D3DCreateBlob(vbSize, &vertex_buffer_cpu);
    JojEngine::Engine::renderer->allocate_resource_in_cpu(vbSize, &vertex_buffer_cpu);

    // Allocate (3 parameters) - GPU
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::UPLOAD, vbSize, &vertexBufferUpload);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::GPU, vbSize, &vertexBufferGPU);

    // Copy (3 parameters)
    //CopyMemory(vertex_buffer_cpu->GetBufferPointer(), vertices, vbSize);
    JojEngine::Engine::renderer->copy_verts_to_cpu_blob(vertices, vbSize, vertex_buffer_cpu);

    // Copy (4 parameters)
    JojEngine::Engine::renderer->copy_verts_to_gpu(vertices, vbSize, vertexBufferUpload, vertexBufferGPU);
    

    // Build Root Signature

    // descri��o para uma assinatura vazia
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // serializa assinatura raiz
    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* error = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &error));

    // cria uma assinatura raiz vazia
    ThrowIfFailed(JojEngine::Engine::renderer->get_device()->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)));

    // --------------------
    // ----- Shaders ------
    // --------------------

    ID3DBlob* vertexShader;
    ID3DBlob* pixelShader;

    D3DReadFileToBlob(L"../vertex.cso", &vertexShader);
    D3DReadFileToBlob(L"../pixel.cso", &pixelShader);

    // --------------------
    // ---- Rasterizer ----
    // --------------------

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

    D3D12_BLEND_DESC blender = {};
    blender.AlphaToCoverageEnable = FALSE;
    blender.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blender.RenderTarget[i] = defaultRenderTargetBlendDesc;

    // ---------------------
    // --- Depth Stencil ---
    // ---------------------

    D3D12_DEPTH_STENCIL_DESC depthStencil = {};
    depthStencil.DepthEnable = TRUE;
    depthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencil.StencilEnable = FALSE;
    depthStencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    depthStencil.FrontFace = defaultStencilOp;
    depthStencil.BackFace = defaultStencilOp;

    // -----------------------------------
    // --- Pipeline State Object (PSO) ---
    // -----------------------------------

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = rootSignature;
    pso.VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
    pso.PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    pso.BlendState = blender;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = rasterizer;
    pso.DepthStencilState = depthStencil;
    pso.InputLayout = { input_layout, 2 };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso.SampleDesc.Count = JojEngine::Engine::dx12_graphics->get_antialiasing();
    pso.SampleDesc.Quality = JojEngine::Engine::dx12_graphics->get_quality();
    JojEngine::Engine::dx12_graphics->get_device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipelineState));

    vertexShader->Release();
    pixelShader->Release();

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
    JojEngine::Engine::renderer->clear(pipelineState);

    // submete comandos de configura��o do pipeline
    JojEngine::Engine::dx12_graphics->get_command_list()->SetGraphicsRootSignature(rootSignature);

    vertexBufferView.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = vertexByteStride;
    vertexBufferView.SizeInBytes = vertexBufferSize;

    JojEngine::Engine::dx12_graphics->get_command_list()->IASetVertexBuffers(0, 1, &vertexBufferView);
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // submete comandos de desenho
    JojEngine::Engine::dx12_graphics->get_command_list()->DrawInstanced(6, 1, 0, 0);

    JojEngine::Engine::renderer->present();
}

void MyGame::shutdown()
{
    rootSignature->Release();
    pipelineState->Release();
}