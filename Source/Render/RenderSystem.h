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

        bool IsInitialized() const { return mInitialized; }

        std::uint32_t GetWidth() const;
        std::uint32_t GetHeight() const;

    private:
        bool mInitialized = false;

        FRenderSystemDesc mDesc {};

        std::unique_ptr<DX11Device> mDevice;
        std::unique_ptr<DebugRenderer> mDebugRenderer;
    };
}