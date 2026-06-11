#include "DebugRenderer.h"

#include "DX11/DX11Device.h"

#include <Windows.h>
#include <cstddef>
#include <cstring>
#include <d3dcompiler.h>

namespace Render
{
    namespace
    {
        constexpr const char* DebugTriangleShaderSource = R"(
cbuffer DebugViewConstants : register(b0)
{
    row_major float4x4 ViewProjectionMatrix;
};

struct VSInput
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = mul(float4(input.Position, 1.0f), ViewProjectionMatrix);
    output.Color = input.Color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.Color;
}
)";

        Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
            const char* source,
            const char* entryPoint,
            const char* target
        )
        {
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

            UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
            compileFlags |= D3DCOMPILE_DEBUG;
            compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            HRESULT hr = D3DCompile(
                source,
                std::strlen(source),
                "DebugRenderer",
                nullptr,
                nullptr,
                entryPoint,
                target,
                compileFlags,
                0,
                shaderBlob.GetAddressOf(),
                errorBlob.GetAddressOf()
            );

            if (FAILED(hr))
            {
                if (errorBlob)
                {
                    OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                    OutputDebugStringA("\n");
                }

                return nullptr;
            }

            return shaderBlob;
        }
    }

    DebugRenderer::~DebugRenderer()
    {
        Shutdown();
    }

    bool DebugRenderer::Initialize(DX11Device& device)
    {
        Shutdown();

        ID3D11Device* d3dDevice = device.GetDevice();

        if (!d3dDevice)
            return false;

        Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob =
            CompileShader(DebugTriangleShaderSource, "VSMain", "vs_5_0");

        if (!vertexShaderBlob)
            return false;

        Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob =
            CompileShader(DebugTriangleShaderSource, "PSMain", "ps_5_0");

        if (!pixelShaderBlob)
            return false;

        HRESULT hr = d3dDevice->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            mVertexShader.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        hr = d3dDevice->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            mPixelShader.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                offsetof(FDebugVertex, Position),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                offsetof(FDebugVertex, Color),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };

        hr = d3dDevice->CreateInputLayout(
            inputElements,
            static_cast<UINT>(sizeof(inputElements) / sizeof(inputElements[0])),
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            mInputLayout.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        const FDebugVertex vertices[] =
        {
            { {  0.0f,  2.35f, 4.0f }, { 1.0f, 0.15f, 0.10f, 1.0f } },
            { {  1.0f,  1.00f, 4.0f }, { 0.10f, 1.0f, 0.25f, 1.0f } },
            { { -1.0f,  1.00f, 4.0f }, { 0.15f, 0.35f, 1.0f, 1.0f } }
        };

        D3D11_BUFFER_DESC vertexBufferDesc {};
        vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(vertices));
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = sizeof(FDebugVertex);

        D3D11_SUBRESOURCE_DATA vertexInitialData {};
        vertexInitialData.pSysMem = vertices;
        vertexInitialData.SysMemPitch = 0;
        vertexInitialData.SysMemSlicePitch = 0;

        hr = d3dDevice->CreateBuffer(
            &vertexBufferDesc,
            &vertexInitialData,
            mVertexBuffer.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        D3D11_BUFFER_DESC constantBufferDesc {};
        constantBufferDesc.ByteWidth = static_cast<UINT>(sizeof(FDebugViewConstants));
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantBufferDesc.MiscFlags = 0;
        constantBufferDesc.StructureByteStride = 0;

        hr = d3dDevice->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            mViewConstantBuffer.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        mInitialized = true;
        return true;
    }

    void DebugRenderer::Shutdown()
    {
        mViewConstantBuffer.Reset();
        mVertexBuffer.Reset();
        mInputLayout.Reset();
        mPixelShader.Reset();
        mVertexShader.Reset();

        mInitialized = false;
    }

    void DebugRenderer::DrawDebugTriangle(DX11Device& device, const FRenderViewInfo& viewInfo)
    {
        if (!mInitialized)
            return;

        ID3D11DeviceContext* context = device.GetContext();

        if (!context)
            return;

        if (!mViewConstantBuffer)
            return;

        D3D11_MAPPED_SUBRESOURCE mappedResource {};

        HRESULT hr = context->Map(
            mViewConstantBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedResource
        );

        if (FAILED(hr))
            return;

        FDebugViewConstants* constants =
            static_cast<FDebugViewConstants*>(mappedResource.pData);

        constants->ViewProjectionMatrix = viewInfo.ViewProjectionMatrix;

        context->Unmap(mViewConstantBuffer.Get(), 0);

        UINT stride = sizeof(FDebugVertex);
        UINT offset = 0;

        ID3D11Buffer* vertexBuffers[] =
        {
            mVertexBuffer.Get()
        };

        ID3D11Buffer* constantBuffers[] =
        {
            mViewConstantBuffer.Get()
        };

        context->IASetInputLayout(mInputLayout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);

        context->VSSetShader(mVertexShader.Get(), nullptr, 0);
        context->VSSetConstantBuffers(0, 1, constantBuffers);

        context->PSSetShader(mPixelShader.Get(), nullptr, 0);

        context->Draw(3, 0);
    }
}