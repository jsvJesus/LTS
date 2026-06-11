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

        mDebugLines.reserve(512);

        mInitialized = true;
        return true;
    }

    void DebugRenderer::Shutdown()
    {
        mDebugLines.clear();

        mViewConstantBuffer.Reset();
        mDynamicLineVertexBuffer.Reset();
        mInputLayout.Reset();
        mPixelShader.Reset();
        mVertexShader.Reset();

        mInitialized = false;
    }

    void DebugRenderer::ClearDebugDraw()
    {
        mDebugLines.clear();
    }

    bool DebugRenderer::AddDebugLine(
        const Core::Vector3& start,
        const Core::Vector3& end,
        const FRenderColor& color
    )
    {
        if ((mDebugLines.size() + 1) * 2 > MaxDebugLineVertices)
            return false;

        FDebugLine line {};
        line.Start = start;
        line.End = end;
        line.Color = color;

        mDebugLines.push_back(line);
        return true;
    }

    void DebugRenderer::AddDebugGrid(
        Core::i32 halfSize,
        Core::f32 spacing,
        const FRenderColor& lineColor,
        const FRenderColor& centerLineColor
    )
    {
        if (halfSize < 1)
            halfSize = 1;

        if (spacing <= 0.001f)
            spacing = 1.0f;

        const Core::f32 gridExtent = static_cast<Core::f32>(halfSize) * spacing;

        for (Core::i32 lineIndex = -halfSize; lineIndex <= halfSize; ++lineIndex)
        {
            const Core::f32 coordinate = static_cast<Core::f32>(lineIndex) * spacing;
            const FRenderColor& color = lineIndex == 0 ? centerLineColor : lineColor;

            AddDebugLine(
                Core::Vector3(-gridExtent, 0.0f, coordinate),
                Core::Vector3( gridExtent, 0.0f, coordinate),
                color
            );

            AddDebugLine(
                Core::Vector3(coordinate, 0.0f, -gridExtent),
                Core::Vector3(coordinate, 0.0f,  gridExtent),
                color
            );
        }
    }

    void DebugRenderer::AddDebugAxes(Core::f32 length)
    {
        if (length <= 0.001f)
            length = 1.0f;

        FRenderColor axisXColor {};
        axisXColor.R = 1.00f;
        axisXColor.G = 0.15f;
        axisXColor.B = 0.10f;
        axisXColor.A = 1.0f;

        FRenderColor axisYColor {};
        axisYColor.R = 0.15f;
        axisYColor.G = 1.00f;
        axisYColor.B = 0.20f;
        axisYColor.A = 1.0f;

        FRenderColor axisZColor {};
        axisZColor.R = 0.15f;
        axisZColor.G = 0.35f;
        axisZColor.B = 1.00f;
        axisZColor.A = 1.0f;

        AddDebugLine(
            Core::Vector3(0.0f, 0.02f, 0.0f),
            Core::Vector3(length, 0.02f, 0.0f),
            axisXColor
        );

        AddDebugLine(
            Core::Vector3(0.0f, 0.02f, 0.0f),
            Core::Vector3(0.0f, length, 0.0f),
            axisYColor
        );

        AddDebugLine(
            Core::Vector3(0.0f, 0.02f, 0.0f),
            Core::Vector3(0.0f, 0.02f, length),
            axisZColor
        );
    }

    void DebugRenderer::AddDebugWireTriangle(
        const Core::Vector3& a,
        const Core::Vector3& b,
        const Core::Vector3& c,
        const FRenderColor& color
    )
    {
        AddDebugLine(a, b, color);
        AddDebugLine(b, c, color);
        AddDebugLine(c, a, color);
    }

    void DebugRenderer::DrawDebugPrimitives(DX11Device& device, const FRenderViewInfo& viewInfo)
    {
        if (!mInitialized)
            return;

        if (mDebugLines.empty())
            return;

        ID3D11DeviceContext* context = device.GetContext();

        if (!context)
            return;

        if (!mDynamicLineVertexBuffer || !mViewConstantBuffer)
            return;

        std::vector<FDebugVertex> vertices;
        vertices.reserve(mDebugLines.size() * 2);

        for (const FDebugLine& line : mDebugLines)
        {
            if (vertices.size() + 2 > MaxDebugLineVertices)
                break;

            FDebugVertex startVertex {};
            startVertex.Position[0] = line.Start.X;
            startVertex.Position[1] = line.Start.Y;
            startVertex.Position[2] = line.Start.Z;
            startVertex.Color[0] = line.Color.R;
            startVertex.Color[1] = line.Color.G;
            startVertex.Color[2] = line.Color.B;
            startVertex.Color[3] = line.Color.A;

            FDebugVertex endVertex {};
            endVertex.Position[0] = line.End.X;
            endVertex.Position[1] = line.End.Y;
            endVertex.Position[2] = line.End.Z;
            endVertex.Color[0] = line.Color.R;
            endVertex.Color[1] = line.Color.G;
            endVertex.Color[2] = line.Color.B;
            endVertex.Color[3] = line.Color.A;

            vertices.push_back(startVertex);
            vertices.push_back(endVertex);
        }

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