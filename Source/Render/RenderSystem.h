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

        void SetClearColor(const FRenderColor& color);
        [[nodiscard]] const FRenderColor& GetClearColor() const;

        bool SetDebugRenderingEnabled(bool enabled);
        bool ToggleDebugRendering();

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }
        [[nodiscard]] bool IsDebugRendererAvailable() const;
        [[nodiscard]] bool IsDebugRenderingEnabled() const { return mDebugRenderingEnabled; }

        std::uint32_t GetWidth() const;
        std::uint32_t GetHeight() const;

    private:
        bool mInitialized = false;
        bool mDebugRenderingEnabled = false;

        FRenderSystemDesc mDesc {};

        std::unique_ptr<DX11Device> mDevice;
        std::unique_ptr<DebugRenderer> mDebugRenderer;
    };
}