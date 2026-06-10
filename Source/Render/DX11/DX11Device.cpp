#include "DX11Device.h"

#include <iterator>
#include <Windows.h>

namespace Render
{
    DX11Device::~DX11Device()
    {
        Shutdown();
    }

    bool DX11Device::Initialize(
        void* nativeWindowHandle,
        std::uint32_t width,
        std::uint32_t height,
        bool enableDebugLayer
    )
    {
        Shutdown();

        if (!nativeWindowHandle)
            return false;

        if (width == 0 || height == 0)
            return false;

        if (!CreateDeviceAndSwapChain(nativeWindowHandle, width, height, enableDebugLayer))
            return false;

        if (!CreateBackBufferResources())
        {
            Shutdown();
            return false;
        }

        if (!CreateDepthStencilResources())
        {
            Shutdown();
            return false;
        }

        mWidth = width;
        mHeight = height;
        mInitialized = true;

        return true;
    }

    void DX11Device::Shutdown()
    {
        if (mContext)
        {
            mContext->ClearState();
            mContext->Flush();
        }

        ReleaseBackBufferResources();

        mSwapChain.Reset();
        mContext.Reset();
        mDevice.Reset();

        mWidth = 0;
        mHeight = 0;
        mInitialized = false;
    }

    bool DX11Device::CreateDeviceAndSwapChain(
        void* nativeWindowHandle,
        std::uint32_t width,
        std::uint32_t height,
        bool enableDebugLayer
    )
    {
        HWND hwnd = static_cast<HWND>(nativeWindowHandle);

        DXGI_SWAP_CHAIN_DESC swapChainDesc {};
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        if (enableDebugLayer)
            createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        D3D_FEATURE_LEVEL selectedFeatureLevel {};

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createFlags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            mSwapChain.GetAddressOf(),
            mDevice.GetAddressOf(),
            &selectedFeatureLevel,
            mContext.GetAddressOf()
        );

#if defined(_DEBUG)
        if (FAILED(hr) && (createFlags & D3D11_CREATE_DEVICE_DEBUG))
        {
            createFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

            hr = D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                createFlags,
                featureLevels,
                static_cast<UINT>(std::size(featureLevels)),
                D3D11_SDK_VERSION,
                &swapChainDesc,
                mSwapChain.GetAddressOf(),
                mDevice.GetAddressOf(),
                &selectedFeatureLevel,
                mContext.GetAddressOf()
            );
        }
#endif

        return SUCCEEDED(hr);
    }

    bool DX11Device::CreateBackBufferResources()
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

        HRESULT hr = mSwapChain->GetBuffer(
            0,
            IID_PPV_ARGS(backBuffer.GetAddressOf())
        );

        if (FAILED(hr))
            return false;

        hr = mDevice->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            mBackBufferRTV.GetAddressOf()
        );

        return SUCCEEDED(hr);
    }

    bool DX11Device::CreateDepthStencilResources()
    {
        D3D11_TEXTURE2D_DESC depthDesc {};
        depthDesc.Width = mWidth;
        depthDesc.Height = mHeight;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDesc.CPUAccessFlags = 0;
        depthDesc.MiscFlags = 0;

        HRESULT hr = mDevice->CreateTexture2D(
            &depthDesc,
            nullptr,
            mDepthStencilBuffer.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        hr = mDevice->CreateDepthStencilView(
            mDepthStencilBuffer.Get(),
            nullptr,
            mDepthStencilView.GetAddressOf()
        );

        return SUCCEEDED(hr);
    }

    void DX11Device::ReleaseBackBufferResources()
    {
        mDepthStencilView.Reset();
        mDepthStencilBuffer.Reset();
        mBackBufferRTV.Reset();
    }

    void DX11Device::BeginFrame(const FRenderColor& clearColor)
    {
        if (!mInitialized)
            return;

        ID3D11RenderTargetView* renderTargets[] =
        {
            mBackBufferRTV.Get()
        };

        mContext->OMSetRenderTargets(
            1,
            renderTargets,
            mDepthStencilView.Get()
        );

        D3D11_VIEWPORT viewport {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(mWidth);
        viewport.Height = static_cast<float>(mHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        mContext->RSSetViewports(1, &viewport);

        const float color[] =
        {
            clearColor.R,
            clearColor.G,
            clearColor.B,
            clearColor.A
        };

        mContext->ClearRenderTargetView(mBackBufferRTV.Get(), color);

        mContext->ClearDepthStencilView(
            mDepthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0
        );
    }

    void DX11Device::Present(bool enableVSync)
    {
        if (!mInitialized || !mSwapChain)
            return;

        mSwapChain->Present(enableVSync ? 1 : 0, 0);
    }

    bool DX11Device::Resize(std::uint32_t width, std::uint32_t height)
    {
        if (!mInitialized)
            return false;

        if (width == 0 || height == 0)
            return true;

        if (width == mWidth && height == mHeight)
            return true;

        if (mContext)
            mContext->OMSetRenderTargets(0, nullptr, nullptr);

        ReleaseBackBufferResources();

        HRESULT hr = mSwapChain->ResizeBuffers(
            0,
            width,
            height,
            DXGI_FORMAT_UNKNOWN,
            0
        );

        if (FAILED(hr))
            return false;

        mWidth = width;
        mHeight = height;

        if (!CreateBackBufferResources())
            return false;

        if (!CreateDepthStencilResources())
            return false;

        return true;
    }
}