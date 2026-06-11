#pragma once

#include <cstdint>

#include <d3d11.h>
#include <wrl/client.h>

#include "RHI/RenderTypes.h"

namespace Render
{
    class DX11Device;

    class DebugRenderer final
    {
    public:
        DebugRenderer() = default;
        ~DebugRenderer();

        DebugRenderer(const DebugRenderer&) = delete;
        DebugRenderer& operator=(const DebugRenderer&) = delete;

        bool Initialize(DX11Device& device);
        void Shutdown();

        void DrawDebugPrimitives(DX11Device& device, const FRenderViewInfo& viewInfo);

    private:
        static constexpr std::uint32_t MaxDebugLineVertices = 65536;

        struct FDebugVertex
        {
            float Position[3];
            float Color[4];
        };

        struct FDebugViewConstants
        {
            Core::Matrix4 ViewProjectionMatrix;
        };

    private:
        bool mInitialized = false;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;

        Microsoft::WRL::ComPtr<ID3D11Buffer> mDynamicLineVertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> mViewConstantBuffer;
    };
}