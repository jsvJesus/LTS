#include "RenderSystem.h"

#include "DX11/DX11Device.h"
#include "DebugRenderer.h"

namespace Render
{
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

        mInitialized = false;
    }

    void RenderSystem::BeginFrame(const FRenderFrameInfo& frameInfo)
    {
        (void)frameInfo;

        if (!mInitialized || !mDevice)
            return;

        mDevice->BeginFrame(mDesc.ClearColor);
    }

    void RenderSystem::RenderDebug()
    {
        if (!mInitialized || !mDevice || !mDebugRenderer)
            return;

        mDebugRenderer->DrawDebugTriangle(*mDevice);
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

    std::uint32_t RenderSystem::GetWidth() const
    {
        return mDevice ? mDevice->GetWidth() : 0;
    }

    std::uint32_t RenderSystem::GetHeight() const
    {
        return mDevice ? mDevice->GetHeight() : 0;
    }
}