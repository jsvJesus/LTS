#include "Render/DX11/DX11Device.h"

#include "Core/Logger.h"

#include <Windows.h>

#include <string>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

namespace Render
{
    DX11Device::~DX11Device()
    {
        Shutdown();
    }

    bool DX11Device::Initialize(const DX11DeviceCreateInfo& createInfo)
    {
        if (mInitialized)
        {
            Core::Logger::Warning("DX11Device", "Initialize called, but DX11 device is already initialized.");
            return true;
        }

        if (createInfo.NativeWindowHandle == nullptr)
        {
            Core::Logger::Error("DX11Device", "Native window handle is null.");
            return false;
        }

        if (createInfo.Width <= 0 || createInfo.Height <= 0)
        {
            Core::Logger::Error("DX11Device", "Invalid backbuffer size.");
            return false;
        }

        mNativeWindowHandle = createInfo.NativeWindowHandle;
        mBackBufferWidth = createInfo.Width;
        mBackBufferHeight = createInfo.Height;
        mEnableVSync = createInfo.EnableVSync;

        Core::Logger::Info("DX11Device", "Creating DX11 device.");

        bool deviceCreated = false;

        if (createInfo.EnableDebugLayer)
        {
            deviceCreated = CreateDeviceAndSwapChain(createInfo, true);

            if (!deviceCreated)
            {
                Core::Logger::Warning("DX11Device", "DX11 debug layer failed. Retrying without debug layer.");
            }
        }

        if (!deviceCreated)
        {
            deviceCreated = CreateDeviceAndSwapChain(createInfo, false);
        }

        if (!deviceCreated)
        {
            Core::Logger::Fatal("DX11Device", "Failed to create DX11 device and swapchain.");
            Shutdown();
            return false;
        }

        if (!CreateBackBufferRenderTarget())
        {
            Core::Logger::Fatal("DX11Device", "Failed to create backbuffer render target.");
            Shutdown();
            return false;
        }

        mInitialized = true;

        Core::Logger::Info("DX11Device", "DX11 device initialized.");

        return true;
    }

    void DX11Device::Shutdown()
    {
        if (!mInitialized &&
            mDevice == nullptr &&
            mDeviceContext == nullptr &&
            mSwapChain == nullptr)
        {
            return;
        }

        Core::Logger::Info("DX11Device", "DX11 device shutdown started.");

        ReleaseBackBufferRenderTarget();

        if (mDeviceContext)
        {
            mDeviceContext->ClearState();
            mDeviceContext->Flush();
        }

        mSwapChain.Reset();
        mDeviceContext.Reset();
        mDevice.Reset();

        mNativeWindowHandle = nullptr;
        mBackBufferWidth = 0;
        mBackBufferHeight = 0;
        mInitialized = false;

        Core::Logger::Info("DX11Device", "DX11 device shutdown finished.");
    }

    void DX11Device::BeginFrame(const ClearColor& clearColor)
    {
        if (!mInitialized || !mDeviceContext || !mBackBufferRenderTargetView)
        {
            return;
        }

        const float color[4] =
        {
            clearColor.R,
            clearColor.G,
            clearColor.B,
            clearColor.A
        };

        ID3D11RenderTargetView* renderTargets[] =
        {
            mBackBufferRenderTargetView.Get()
        };

        mDeviceContext->OMSetRenderTargets(1, renderTargets, nullptr);

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(mBackBufferWidth);
        viewport.Height = static_cast<float>(mBackBufferHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        mDeviceContext->RSSetViewports(1, &viewport);
        mDeviceContext->ClearRenderTargetView(mBackBufferRenderTargetView.Get(), color);
    }

    void DX11Device::EndFrame()
    {
        if (!mInitialized || !mSwapChain)
        {
            return;
        }

        const UINT syncInterval = mEnableVSync ? 1u : 0u;
        const UINT presentFlags = 0u;

        const HRESULT result = mSwapChain->Present(syncInterval, presentFlags);

        if (FAILED(result))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("SwapChain Present failed.", result));
        }
    }

    bool DX11Device::Resize(const Core::i32 width, const Core::i32 height)
    {
        if (!mInitialized || !mSwapChain)
        {
            return false;
        }

        if (width <= 0 || height <= 0)
        {
            return false;
        }

        if (width == mBackBufferWidth && height == mBackBufferHeight)
        {
            return true;
        }

        Core::Logger::Info(
            "DX11Device",
            "Resizing swapchain to " + std::to_string(width) + "x" + std::to_string(height)
        );

        ReleaseBackBufferRenderTarget();

        if (mDeviceContext)
        {
            mDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
            mDeviceContext->Flush();
        }

        const HRESULT result = mSwapChain->ResizeBuffers(
            0,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            DXGI_FORMAT_UNKNOWN,
            0
        );

        if (FAILED(result))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("SwapChain ResizeBuffers failed.", result));
            return false;
        }

        mBackBufferWidth = width;
        mBackBufferHeight = height;

        return CreateBackBufferRenderTarget();
    }

    bool DX11Device::ResizeIfNeeded(const Core::i32 width, const Core::i32 height)
    {
        if (width <= 0 || height <= 0)
        {
            return false;
        }

        if (width == mBackBufferWidth && height == mBackBufferHeight)
        {
            return true;
        }

        return Resize(width, height);
    }

    bool DX11Device::IsInitialized() const
    {
        return mInitialized;
    }

    Core::i32 DX11Device::GetBackBufferWidth() const
    {
        return mBackBufferWidth;
    }

    Core::i32 DX11Device::GetBackBufferHeight() const
    {
        return mBackBufferHeight;
    }

    ID3D11Device* DX11Device::GetDevice() const
    {
        return mDevice.Get();
    }

    ID3D11DeviceContext* DX11Device::GetDeviceContext() const
    {
        return mDeviceContext.Get();
    }

    IDXGISwapChain* DX11Device::GetSwapChain() const
    {
        return mSwapChain.Get();
    }

    bool DX11Device::CreateDeviceAndSwapChain(const DX11DeviceCreateInfo& createInfo, const bool enableDebugLayer)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc{};
        swapChainDesc.BufferDesc.Width = static_cast<UINT>(createInfo.Width);
        swapChainDesc.BufferDesc.Height = static_cast<UINT>(createInfo.Height);
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.OutputWindow = static_cast<HWND>(createInfo.NativeWindowHandle);
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        UINT deviceFlags = 0;

        if (enableDebugLayer)
        {
            deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        const D3D_FEATURE_LEVEL requestedFeatureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        D3D_FEATURE_LEVEL createdFeatureLevel{};

        const HRESULT result = ::D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            deviceFlags,
            requestedFeatureLevels,
            static_cast<UINT>(std::size(requestedFeatureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            mSwapChain.GetAddressOf(),
            mDevice.GetAddressOf(),
            &createdFeatureLevel,
            mDeviceContext.GetAddressOf()
        );

        if (FAILED(result))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("D3D11CreateDeviceAndSwapChain failed.", result));
            return false;
        }

        Core::Logger::Info("DX11Device", "D3D11 device created.");

        return true;
    }

    bool DX11Device::CreateBackBufferRenderTarget()
    {
        if (!mSwapChain || !mDevice)
        {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

        const HRESULT getBufferResult = mSwapChain->GetBuffer(
            0,
            __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(backBuffer.GetAddressOf())
        );

        if (FAILED(getBufferResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("SwapChain GetBuffer failed.", getBufferResult));
            return false;
        }

        const HRESULT createViewResult = mDevice->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            mBackBufferRenderTargetView.GetAddressOf()
        );

        if (FAILED(createViewResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("CreateRenderTargetView failed.", createViewResult));
            return false;
        }

        Core::Logger::Info("DX11Device", "Backbuffer render target created.");

        return true;
    }

    void DX11Device::ReleaseBackBufferRenderTarget()
    {
        if (mBackBufferRenderTargetView)
        {
            mBackBufferRenderTargetView.Reset();
        }
    }

    Core::String DX11Device::FormatHRESULT(const char* message, const long result)
    {
        Core::String text = message;
        text += " HRESULT=0x";

        char buffer[16]{};
        sprintf_s(buffer, "%08lX", static_cast<unsigned long>(result));

        text += buffer;

        return text;
    }
}