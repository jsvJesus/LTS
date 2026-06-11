#pragma once

#include <cstdint>
#include <vector>

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

        void ClearDebugDraw();

        bool AddDebugLine(
            const Core::Vector3& start,
            const Core::Vector3& end,
            const FRenderColor& color
        );

        void AddDebugGrid(
            Core::i32 halfSize,
            Core::f32 spacing,
            const FRenderColor& lineColor,
            const FRenderColor& centerLineColor
        );

        void AddDebugAxes(Core::f32 length);

        void AddDebugWireTriangle(
            const Core::Vector3& a,
            const Core::Vector3& b,
            const Core::Vector3& c,
            const FRenderColor& color
        );

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

        std::vector<FDebugLine> mDebugLines;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;

        Microsoft::WRL::ComPtr<ID3D11Buffer> mDynamicLineVertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> mViewConstantBuffer;
    };
}