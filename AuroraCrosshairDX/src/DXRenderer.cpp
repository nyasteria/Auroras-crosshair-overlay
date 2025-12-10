#include "DXRenderer.h"
#include <stdexcept>
#include <cstdint>
#include <cstring>

// Vertex Shader
const char* vertexShaderSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float4x4 world;
        float4x4 view;
        float4x4 projection;
        float4 color;
    };

    struct VS_INPUT
    {
        float3 position : POSITION;
        float2 texCoord : TEXCOORD0;
    };

    struct PS_INPUT
    {
        float4 position : SV_POSITION;
        float2 texCoord : TEXCOORD0;
    };

    PS_INPUT main(VS_INPUT input)
    {
        PS_INPUT output;
        
        // Just pass through the position - it's already in NDC space
        output.position = float4(input.position, 1.0f);
        output.texCoord = input.texCoord;
        
        return output;
    }
)";

// Pixel Shader
const char* pixelShaderSource = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float4x4 world;
        float4x4 view;
        float4x4 projection;
        float4 color;
    };

    struct PS_INPUT
    {
        float4 position : SV_POSITION;
        float2 texCoord : TEXCOORD0;
    };

    float4 main(PS_INPUT input) : SV_TARGET
    {
        // Output white with full alpha for testing
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
)";

DXRenderer::DXRenderer()
    : featureLevel(D3D_FEATURE_LEVEL_11_0), indexCount(0)
{
}

DXRenderer::~DXRenderer()
{
    Shutdown();
}

bool DXRenderer::Initialize(HWND hWnd)
{
    try
    {
        if (!CreateSwapChain(hWnd))
            return false;

        if (!CreateShaders())
            return false;

        if (!CreateVertexBuffer())
            return false;

        if (!CreateConstantBuffer())
            return false;

        // Create blend state for alpha blending
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        if (FAILED(device->CreateBlendState(&blendDesc, &blendState)))
            return false;

        context->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);

        // Create sampler state
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = -FLT_MAX;
        samplerDesc.MaxLOD = FLT_MAX;

        if (FAILED(device->CreateSamplerState(&samplerDesc, &samplerState)))
            return false;

        context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void DXRenderer::Shutdown()
{
    if (context)
        context->ClearState();
}

bool DXRenderer::CreateSwapChain(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = 1920;
    swapChainDesc.BufferDesc.Height = 1080;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, &featureLevel, &context)))
    {
        // Try WARP driver as fallback
        if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
            D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, &featureLevel, &context)))
        {
            return false;
        }
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer)))
        return false;

    if (FAILED(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView)))
        return false;

    // Create depth stencil
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = 1920;
    depthStencilDesc.Height = 1080;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    if (FAILED(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer)))
        return false;

    if (FAILED(device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, &depthStencilView)))
        return false;

    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 1920;
    viewport.Height = 1080;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    context->RSSetViewports(1, &viewport);

    return true;
}

bool DXRenderer::CreateShaders()
{
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    if (FAILED(D3DCompile(vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr,
        "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &vsBlob, &errorBlob)))
    {
        return false;
    }

    if (FAILED(D3DCompile(pixelShaderSource, strlen(pixelShaderSource), nullptr, nullptr, nullptr,
        "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &psBlob, &errorBlob)))
    {
        return false;
    }

    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader)))
        return false;

    if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader)))
        return false;

    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (FAILED(device->CreateInputLayout(inputLayout, 2, vsBlob->GetBufferPointer(), 
        vsBlob->GetBufferSize(), &this->inputLayout)))
        return false;

    context->IASetInputLayout(this->inputLayout.Get());
    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);

    return true;
}

bool DXRenderer::CreateVertexBuffer()
{
    VertexData vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) }
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(vertices);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer)))
        return false;

    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
    indexCount = 6;

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer)))
        return false;

    unsigned int stride = sizeof(VertexData);
    unsigned int offset = 0;

    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return true;
}

bool DXRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer)))
        return false;

    context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    return true;
}

bool DXRenderer::LoadPNGTexture(const wchar_t* filePath)
{
    // Load PNG using Windows Imaging Component (WIC)
    IWICImagingFactory* pFactory = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, 
                                   IID_IWICImagingFactory, (void**)&pFactory);
    if (FAILED(hr))
        return false;

    hr = pFactory->CreateDecoderFromFilename(filePath, nullptr, GENERIC_READ, 
                                              WICDecodeMetadataCacheOnLoad, &pDecoder);
    if (FAILED(hr))
    {
        pFactory->Release();
        return false;
    }

    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr))
    {
        pDecoder->Release();
        pFactory->Release();
        return false;
    }

    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr))
    {
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        return false;
    }

    hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPRGBA, 
                                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr))
    {
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        return false;
    }

    UINT width = 0, height = 0;
    pConverter->GetSize(&width, &height);

    UINT stride = width * 4;
    UINT imageSize = stride * height;
    uint8_t* imageData = new uint8_t[imageSize];

    hr = pConverter->CopyPixels(nullptr, stride, imageSize, imageData);
    if (FAILED(hr))
    {
        delete[] imageData;
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        return false;
    }

    // Create texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = imageData;
    initData.SysMemPitch = stride;

    bool success = SUCCEEDED(device->CreateTexture2D(&textureDesc, &initData, &crosshairTexture));

    delete[] imageData;

    if (!success)
    {
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    if (FAILED(device->CreateShaderResourceView(crosshairTexture.Get(), &srvDesc, &crosshairSRV)))
    {
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        return false;
    }

    context->PSSetShaderResources(0, 1, crosshairSRV.GetAddressOf());

    pConverter->Release();
    pFrame->Release();
    pDecoder->Release();
    pFactory->Release();

    return true;
}

void DXRenderer::RenderCrosshair(float posX, float posY, float scale, 
                                  float r, float g, float b, float a)
{
    if (!context || !vertexBuffer || !indexBuffer)
        return;

    // Re-set vertex/index buffers each frame to ensure they're valid
    unsigned int stride = sizeof(VertexData);
    unsigned int offset = 0;
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        ConstantBuffer* pConstantBuffer = (ConstantBuffer*)mappedResource.pData;

        // Scale down significantly - 0.01 means 1% of screen size (20x20 pixels roughly)
        XMMATRIX world = XMMatrixScaling(0.01f * scale, 0.01f * scale, 1.0f);
        XMMATRIX view = XMMatrixIdentity();
        XMMATRIX projection = XMMatrixIdentity();

        pConstantBuffer->world = XMMatrixTranspose(world);
        pConstantBuffer->view = XMMatrixTranspose(view);
        pConstantBuffer->projection = XMMatrixTranspose(projection);
        pConstantBuffer->color = XMFLOAT4(r, g, b, a);

        context->Unmap(constantBuffer.Get(), 0);
    }

    context->DrawIndexed(indexCount, 0, 0);
}

void DXRenderer::BeginFrame()
{
    if (!context)
        return;

    // Clear with black
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DXRenderer::EndFrame()
{
    if (!swapChain)
        return;

    // Present at the END of the frame
    swapChain->Present(1, 0);
}
