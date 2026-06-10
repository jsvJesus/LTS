#include "EngineLoop.h"

#include "../Platform/Window.h"
#include "../Render/RenderSystem.h"

namespace Engine
{
    EngineLoop::~EngineLoop()
    {
        Shutdown();
    }

    bool EngineLoop::Initialize(const FApplicationDesc& desc)
    {
        Shutdown();

        mWindow = std::make_unique<Platform::Window>();

        if (!mWindow->Create(desc.Title, desc.Width, desc.Height))
        {
            Shutdown();
            return false;
        }

        Render::FRenderSystemDesc renderDesc {};
        renderDesc.NativeWindowHandle = mWindow->GetNativeHandle();
        renderDesc.Width = mWindow->GetClientWidth();
        renderDesc.Height = mWindow->GetClientHeight();

#if defined(_DEBUG)
        renderDesc.EnableDebugLayer = true;
#else
        renderDesc.EnableDebugLayer = false;
#endif

        renderDesc.EnableDebugRenderer = desc.EnableDebugRenderer;
        renderDesc.EnableVSync = desc.EnableVSync;

        renderDesc.ClearColor.R = 0.015f;
        renderDesc.ClearColor.G = 0.016f;
        renderDesc.ClearColor.B = 0.020f;
        renderDesc.ClearColor.A = 1.0f;

        mRenderSystem = std::make_unique<Render::RenderSystem>();

        if (!mRenderSystem->Initialize(renderDesc))
        {
            Shutdown();
            return false;
        }

        mLastFrameTime = std::chrono::steady_clock::now();

        mRunning = true;
        mInitialized = true;

        return true;
    }

    int EngineLoop::Run()
    {
        if (!mInitialized)
            return -1;

        while (mRunning)
        {
            if (!mWindow->ProcessMessages())
            {
                mRunning = false;
                break;
            }

            const auto currentTime = std::chrono::steady_clock::now();

            const std::chrono::duration<double> delta =
                currentTime - mLastFrameTime;

            mLastFrameTime = currentTime;

            const double deltaSeconds = delta.count();

            HandleResize();
            Tick(deltaSeconds);
            RenderFrame(deltaSeconds);

            ++mFrameIndex;
        }

        return 0;
    }

    void EngineLoop::Shutdown()
    {
        mRunning = false;

        if (mRenderSystem)
        {
            mRenderSystem->Shutdown();
            mRenderSystem.reset();
        }

        if (mWindow)
        {
            mWindow->Destroy();
            mWindow.reset();
        }

        mFrameIndex = 0;
        mInitialized = false;
    }

    void EngineLoop::Tick(double deltaSeconds)
    {
        (void)deltaSeconds;

        // Позже тут будут:
        // input update
        // world tick
        // editor tick
        // game tick
    }

    void EngineLoop::RenderFrame(double deltaSeconds)
    {
        if (!mRenderSystem)
            return;

        Render::FRenderFrameInfo frameInfo {};
        frameInfo.DeltaSeconds = deltaSeconds;
        frameInfo.FrameIndex = mFrameIndex;

        mRenderSystem->BeginFrame(frameInfo);

        mRenderSystem->RenderDebug();

        mRenderSystem->EndFrame();
    }

    void EngineLoop::HandleResize()
    {
        if (!mWindow || !mRenderSystem)
            return;

        const std::uint32_t width = mWindow->GetClientWidth();
        const std::uint32_t height = mWindow->GetClientHeight();

        if (width == 0 || height == 0)
            return;

        if (width == mRenderSystem->GetWidth() && height == mRenderSystem->GetHeight())
            return;

        mRenderSystem->Resize(width, height);
    }
}