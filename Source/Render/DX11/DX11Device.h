#pragma once

#include <cstdint>

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "../RHI/RenderTypes.h"

namespace Render
{
    class DX11Device final
    {
    public:
        DX11Device() = default;
        ~DX11Device();

        DX11Device(const DX11Device&) = delete;
        DX11Device& operator=(const DX11Device&) = delete;

        bool Initialize(
            void* nativeWindowHandle,
            std::uint32_t width,
            std::uint32_t height,
            bool enableDebugLayer
        );

        void Shutdown();

        void BeginFrame(const FRenderColor& clearColor);
        void Present(bool enableVSync);

        bool Resize(std::uint32_t width, std::uint32_t height);

        bool IsInitialized() const { return mInitialized; }

        std::uint32_t GetWidth() const { return mWidth; }
        std::uint32_t GetHeight() const { return mHeight; }

        ID3D11Device* GetDevice() const { return mDevice.Get(); }
        ID3D11DeviceContext* GetContext() const { return mContext.Get(); }

        ID3D11RenderTargetView* GetBackBufferRenderTargetView() const { return mBackBufferRTV.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const { return mDepthStencilView.Get(); }

    private:
        bool CreateDeviceAndSwapChain(
            void* nativeWindowHandle,
            std::uint32_t width,
            std::uint32_t height,
            bool enableDebugLayer
        );

        bool CreateBackBufferResources();
        bool CreateDepthStencilResources();

        void ReleaseBackBufferResources();

    private:
        bool mInitialized = false;

        std::uint32_t mWidth = 0;
        std::uint32_t mHeight = 0;

        Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> mContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mBackBufferRTV;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
    };
}