#pragma once

#include "Core/BaseTypes.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

namespace Render
{
    struct DX11DeviceCreateInfo final
    {
        void* NativeWindowHandle = nullptr;

        Core::i32 Width = 1280;
        Core::i32 Height = 720;

        bool EnableDebugLayer = false;
        bool EnableVSync = true;
    };

    struct ClearColor final
    {
        Core::f32 R = 0.05f;
        Core::f32 G = 0.07f;
        Core::f32 B = 0.09f;
        Core::f32 A = 1.0f;
    };

    class DX11Device final
    {
    public:
        DX11Device() = default;
        ~DX11Device();

        DX11Device(const DX11Device&) = delete;
        DX11Device(DX11Device&&) = delete;

        DX11Device& operator=(const DX11Device&) = delete;
        DX11Device& operator=(DX11Device&&) = delete;

        bool Initialize(const DX11DeviceCreateInfo& createInfo);
        void Shutdown();

        void BeginFrame(const ClearColor& clearColor);
        void DrawDebugTriangle();
        void EndFrame();

        bool Resize(Core::i32 width, Core::i32 height);
        bool ResizeIfNeeded(Core::i32 width, Core::i32 height);

        [[nodiscard]] bool IsInitialized() const;

        [[nodiscard]] Core::i32 GetBackBufferWidth() const;
        [[nodiscard]] Core::i32 GetBackBufferHeight() const;

        [[nodiscard]] ID3D11Device* GetDevice() const;
        [[nodiscard]] ID3D11DeviceContext* GetDeviceContext() const;
        [[nodiscard]] IDXGISwapChain* GetSwapChain() const;

    private:
        bool CreateDeviceAndSwapChain(const DX11DeviceCreateInfo& createInfo, bool enableDebugLayer);
        bool CreateBackBufferRenderTarget();
        bool CreateDepthStencilBuffer();
        bool CreateDebugTrianglePipeline();

        void ReleaseBackBufferResources();
        void ReleaseDebugTrianglePipeline();

        static Core::String FormatHRESULT(const char* message, long result);
        static Core::String BlobToString(ID3DBlob* blob);

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> mDeviceContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mBackBufferRenderTargetView;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilTexture;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> mDebugTriangleVertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> mDebugTrianglePixelShader;

        void* mNativeWindowHandle = nullptr;

        Core::i32 mBackBufferWidth = 0;
        Core::i32 mBackBufferHeight = 0;

        bool mInitialized = false;
        bool mEnableVSync = true;
    };
}