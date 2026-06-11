#include "DebugRenderer.h"

#include "DX11/DX11Device.h"

#include <Windows.h>
#include <cstddef>
#include <cstring>
#include <d3dcompiler.h>
#include <vector>

namespace Render
{
    namespace
    {
        constexpr const char* DebugLineShaderSource = R"(
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
            CompileShader(DebugLineShaderSource, "VSMain", "vs_5_0");

        if (!vertexShaderBlob)
            return false;

        Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob =
            CompileShader(DebugLineShaderSource, "PSMain", "ps_5_0");

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

        D3D11_BUFFER_DESC lineVertexBufferDesc {};
        lineVertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(FDebugVertex) * MaxDebugLineVertices);
        lineVertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        lineVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        lineVertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        lineVertexBufferDesc.MiscFlags = 0;
        lineVertexBufferDesc.StructureByteStride = sizeof(FDebugVertex);

        hr = d3dDevice->CreateBuffer(
            &lineVertexBufferDesc,
            nullptr,
            mDynamicLineVertexBuffer.GetAddressOf()
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
        mDynamicLineVertexBuffer.Reset();
        mInputLayout.Reset();
        mPixelShader.Reset();
        mVertexShader.Reset();

        mInitialized = false;
    }

    void DebugRenderer::DrawDebugPrimitives(DX11Device& device, const FRenderViewInfo& viewInfo)
    {
        if (!mInitialized)
            return;

        ID3D11DeviceContext* context = device.GetContext();

        if (!context)
            return;

        if (!mDynamicLineVertexBuffer || !mViewConstantBuffer)
            return;

        std::vector<FDebugVertex> vertices;
        vertices.reserve(256);

        auto addLine =
            [&vertices](const Core::Vector3& start, const Core::Vector3& end, const float color[4])
            {
                if (vertices.size() + 2 > MaxDebugLineVertices)
                    return;

                FDebugVertex startVertex {};
                startVertex.Position[0] = start.X;
                startVertex.Position[1] = start.Y;
                startVertex.Position[2] = start.Z;
                startVertex.Color[0] = color[0];
                startVertex.Color[1] = color[1];
                startVertex.Color[2] = color[2];
                startVertex.Color[3] = color[3];

                FDebugVertex endVertex {};
                endVertex.Position[0] = end.X;
                endVertex.Position[1] = end.Y;
                endVertex.Position[2] = end.Z;
                endVertex.Color[0] = color[0];
                endVertex.Color[1] = color[1];
                endVertex.Color[2] = color[2];
                endVertex.Color[3] = color[3];

                vertices.push_back(startVertex);
                vertices.push_back(endVertex);
            };

        const float gridColor[4] = { 0.22f, 0.22f, 0.22f, 1.0f };
        const float gridCenterColor[4] = { 0.38f, 0.38f, 0.38f, 1.0f };

        const float axisXColor[4] = { 1.00f, 0.15f, 0.10f, 1.0f };
        const float axisYColor[4] = { 0.15f, 1.00f, 0.20f, 1.0f };
        const float axisZColor[4] = { 0.15f, 0.35f, 1.00f, 1.0f };

        const float triangleColor[4] = { 1.00f, 0.85f, 0.20f, 1.0f };

        constexpr int GridHalfSize = 10;
        constexpr float GridSpacing = 1.0f;
        constexpr float GridExtent = static_cast<float>(GridHalfSize) * GridSpacing;

        for (int lineIndex = -GridHalfSize; lineIndex <= GridHalfSize; ++lineIndex)
        {
            const float coordinate = static_cast<float>(lineIndex) * GridSpacing;
            const float* color = lineIndex == 0 ? gridCenterColor : gridColor;

            addLine(
                Core::Vector3(-GridExtent, 0.0f, coordinate),
                Core::Vector3( GridExtent, 0.0f, coordinate),
                color
            );

            addLine(
                Core::Vector3(coordinate, 0.0f, -GridExtent),
                Core::Vector3(coordinate, 0.0f,  GridExtent),
                color
            );
        }

        addLine(
            Core::Vector3(0.0f, 0.02f, 0.0f),
            Core::Vector3(3.0f, 0.02f, 0.0f),
            axisXColor
        );

        addLine(
            Core::Vector3(0.0f, 0.02f, 0.0f),
            Core::Vector3(0.0f, 3.0f, 0.0f),
            axisYColor
        );

        addLine(
            Core::Vector3(0.0f, 0.02f, 0.0f),
            Core::Vector3(0.0f, 0.02f, 3.0f),
            axisZColor
        );

        const Core::Vector3 triangleA( 0.0f, 2.35f, 4.0f);
        const Core::Vector3 triangleB( 1.0f, 1.00f, 4.0f);
        const Core::Vector3 triangleC(-1.0f, 1.00f, 4.0f);

        addLine(triangleA, triangleB, triangleColor);
        addLine(triangleB, triangleC, triangleColor);
        addLine(triangleC, triangleA, triangleColor);

        if (vertices.empty())
            return;

        D3D11_MAPPED_SUBRESOURCE mappedConstants {};

        HRESULT hr = context->Map(
            mViewConstantBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedConstants
        );

        if (FAILED(hr))
            return;

        FDebugViewConstants* constants =
            static_cast<FDebugViewConstants*>(mappedConstants.pData);

        constants->ViewProjectionMatrix = viewInfo.ViewProjectionMatrix;

        context->Unmap(mViewConstantBuffer.Get(), 0);

        D3D11_MAPPED_SUBRESOURCE mappedVertices {};

        hr = context->Map(
            mDynamicLineVertexBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedVertices
        );

        if (FAILED(hr))
            return;

        std::memcpy(
            mappedVertices.pData,
            vertices.data(),
            sizeof(FDebugVertex) * vertices.size()
        );

        context->Unmap(mDynamicLineVertexBuffer.Get(), 0);

        UINT stride = sizeof(FDebugVertex);
        UINT offset = 0;

        ID3D11Buffer* vertexBuffers[] =
        {
            mDynamicLineVertexBuffer.Get()
        };

        ID3D11Buffer* constantBuffers[] =
        {
            mViewConstantBuffer.Get()
        };

        context->IASetInputLayout(mInputLayout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);

        context->VSSetShader(mVertexShader.Get(), nullptr, 0);
        context->VSSetConstantBuffers(0, 1, constantBuffers);

        context->PSSetShader(mPixelShader.Get(), nullptr, 0);

        context->Draw(static_cast<UINT>(vertices.size()), 0);
    }
}