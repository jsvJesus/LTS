#include "RenderSystem.h"

#include "DX11/DX11Device.h"
#include "DebugRenderer.h"

namespace Render
{
    RenderSystem::RenderSystem() = default;

    RenderSystem::~RenderSystem()
    {
        Shutdown();
    }

    bool RenderSystem::Initialize(const FRenderSystemDesc& desc)
    {
        Shutdown();

        if (!desc.NativeWindowHandle)
            return false;

        if (desc.Width == 0 || desc.Height == 0)
            return false;

        if (desc.Backend != ERenderBackend::DX11)
            return false;

        mDesc = desc;
        mDebugRenderingEnabled = false;
        mHasCurrentViewInfo = false;
        mCurrentViewInfo = FRenderViewInfo {};

        mDevice = std::make_unique<DX11Device>();

        if (!mDevice->Initialize(
            desc.NativeWindowHandle,
            desc.Width,
            desc.Height,
            desc.EnableDebugLayer
        ))
        {
            Shutdown();
            return false;
        }

        if (desc.EnableDebugRenderer)
        {
            mDebugRenderer = std::make_unique<DebugRenderer>();

            if (!mDebugRenderer->Initialize(*mDevice))
            {
                Shutdown();
                return false;
            }

            mDebugRenderingEnabled = desc.EnableDebugRendering;
        }

        mInitialized = true;
        return true;
    }

    void RenderSystem::Shutdown()
    {
        if (mDebugRenderer)
        {
            mDebugRenderer->Shutdown();
            mDebugRenderer.reset();
        }

        if (mDevice)
        {
            mDevice->Shutdown();
            mDevice.reset();
        }

        mHasCurrentViewInfo = false;
        mCurrentViewInfo = FRenderViewInfo {};

        mDebugRenderingEnabled = false;
        mInitialized = false;
    }

    void RenderSystem::BeginFrame(const FRenderFrameInfo& frameInfo)
    {
        if (!mInitialized || !mDevice)
            return;

        mHasCurrentViewInfo = frameInfo.HasViewInfo;

        if (frameInfo.HasViewInfo)
        {
            mCurrentViewInfo = frameInfo.ViewInfo;
        }

        ClearDebugDraw();

        mDevice->BeginFrame(mDesc.ClearColor);
    }

    void RenderSystem::RenderDebug()
    {
        if (!mInitialized || !mDevice || !mDebugRenderer)
            return;

        if (!mDebugRenderingEnabled)
            return;

        if (!mHasCurrentViewInfo)
            return;

        mDebugRenderer->DrawDebugPrimitives(*mDevice, mCurrentViewInfo);
    }

    void RenderSystem::EndFrame()
    {
        if (!mInitialized || !mDevice)
            return;

        mDevice->Present(mDesc.EnableVSync);
    }

    bool RenderSystem::Resize(std::uint32_t width, std::uint32_t height)
    {
        if (!mInitialized || !mDevice)
            return false;

        return mDevice->Resize(width, height);
    }

    void RenderSystem::ClearDebugDraw()
    {
        if (!mDebugRenderer)
            return;

        mDebugRenderer->ClearDebugDraw();
    }

    bool RenderSystem::DrawDebugLine(
        const Core::Vector3& start,
        const Core::Vector3& end,
        const FRenderColor& color
    )
    {
        if (!mDebugRenderer)
            return false;

        return mDebugRenderer->AddDebugLine(start, end, color);
    }

    void RenderSystem::DrawDebugGrid(
        const Core::i32 halfSize,
        const Core::f32 spacing,
        const FRenderColor& lineColor,
        const FRenderColor& centerLineColor
    )
    {
        if (!mDebugRenderer)
            return;

        mDebugRenderer->AddDebugGrid(halfSize, spacing, lineColor, centerLineColor);
    }

    void RenderSystem::DrawDebugAxes(const Core::f32 length)
    {
        if (!mDebugRenderer)
            return;

        mDebugRenderer->AddDebugAxes(length);
    }

    void RenderSystem::DrawDebugWireTriangle(
        const Core::Vector3& a,
        const Core::Vector3& b,
        const Core::Vector3& c,
        const FRenderColor& color
    )
    {
        if (!mDebugRenderer)
            return;

        mDebugRenderer->AddDebugWireTriangle(a, b, c, color);
    }

    void RenderSystem::SetClearColor(const FRenderColor& color)
    {
        mDesc.ClearColor = color;
    }

    const FRenderColor& RenderSystem::GetClearColor() const
    {
        return mDesc.ClearColor;
    }

    bool RenderSystem::SetDebugRenderingEnabled(const bool enabled)
    {
        if (!mDebugRenderer)
        {
            mDebugRenderingEnabled = false;
            return false;
        }

        mDebugRenderingEnabled = enabled;
        return true;
    }

    bool RenderSystem::ToggleDebugRendering()
    {
        return SetDebugRenderingEnabled(!mDebugRenderingEnabled);
    }

    bool RenderSystem::IsDebugRendererAvailable() const
    {
        return mDebugRenderer != nullptr;
    }

    std::uint32_t RenderSystem::GetWidth() const
    {
        return mDevice ? mDevice->GetWidth() : 0;
    }

    std::uint32_t RenderSystem::GetHeight() const
    {
        return mDevice ? mDevice->GetHeight() : 0;
    }
}