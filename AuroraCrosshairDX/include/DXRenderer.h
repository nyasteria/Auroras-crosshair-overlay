#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <wincodec.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class DXRenderer
{
public:
    DXRenderer();
    ~DXRenderer();

    bool Initialize(HWND hWnd);
    void Shutdown();
    
    bool LoadPNGTexture(const wchar_t* filePath);
    void BeginFrame();
    void RenderCrosshair(float posX, float posY, float scale = 1.0f, 
                         float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
    void EndFrame();
    
    ID3D11Device* GetDevice() const { return device.Get(); }
    ID3D11DeviceContext* GetContext() const { return context.Get(); }

private:
    struct VertexData
    {
        XMFLOAT3 position;
        XMFLOAT2 texCoord;
    };

    struct ConstantBuffer
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
        XMFLOAT4 color;
    };

    bool CreateSwapChain(HWND hWnd);
    bool CreateShaders();
    bool CreateVertexBuffer();
    bool CreateConstantBuffer();

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ComPtr<ID3D11DepthStencilView> depthStencilView;

    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11Buffer> constantBuffer;

    ComPtr<ID3D11Texture2D> crosshairTexture;
    ComPtr<ID3D11ShaderResourceView> crosshairSRV;
    ComPtr<ID3D11SamplerState> samplerState;
    
    ComPtr<ID3D11BlendState> blendState;
    ComPtr<ID3D11RasterizerState> rasterizerState;

    D3D_FEATURE_LEVEL featureLevel;
    unsigned int indexCount;
};
