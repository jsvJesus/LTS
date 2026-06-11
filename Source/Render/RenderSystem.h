#pragma once

#include <cstdint>
#include <memory>

#include "RHI/RenderTypes.h"

namespace Render
{
    class DX11Device;
    class DebugRenderer;

    class RenderSystem final
    {
    public:
        RenderSystem();
        ~RenderSystem();

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;

        bool Initialize(const FRenderSystemDesc& desc);
        void Shutdown();

        void BeginFrame(const FRenderFrameInfo& frameInfo);
        void RenderDebug();
        void EndFrame();

        bool Resize(std::uint32_t width, std::uint32_t height);

        void ClearDebugDraw();

        bool DrawDebugLine(
            const Core::Vector3& start,
            const Core::Vector3& end,
            const FRenderColor& color
        );

        void DrawDebugGrid(
            Core::i32 halfSize,
            Core::f32 spacing,
            const FRenderColor& lineColor,
            const FRenderColor& centerLineColor
        );

        void DrawDebugAxes(Core::f32 length);

        void DrawDebugWireTriangle(
            const Core::Vector3& a,
            const Core::Vector3& b,
            const Core::Vector3& c,
            const FRenderColor& color
        );

        void SetClearColor(const FRenderColor& color);
        [[nodiscard]] const FRenderColor& GetClearColor() const;

        bool SetDebugRenderingEnabled(bool enabled);
        bool ToggleDebugRendering();

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] bool IsDebugRendererAvailable() const;
        [[nodiscard]] bool IsDebugRenderingEnabled() const { return mDebugRenderingEnabled; }

        [[nodiscard]] bool HasCurrentViewInfo() const { return mHasCurrentViewInfo; }
        [[nodiscard]] const FRenderViewInfo& GetCurrentViewInfo() const { return mCurrentViewInfo; }

        std::uint32_t GetWidth() const;
        std::uint32_t GetHeight() const;

    private:
        bool mInitialized = false;
        bool mDebugRenderingEnabled = false;

        bool mHasCurrentViewInfo = false;
        FRenderViewInfo mCurrentViewInfo {};

        FRenderSystemDesc mDesc {};

        std::unique_ptr<DX11Device> mDevice;
        std::unique_ptr<DebugRenderer> mDebugRenderer;
    };
}