#include "Render/DX11/DX11Device.h"

#include "Core/Logger.h"

#include <Windows.h>
#include <d3dcompiler.h>

#include <cstdio>
#include <cstring>
#include <string>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Render
{
    namespace
    {
        constexpr const char* DebugTriangleShaderSource = R"(
struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    float2 positions[3] =
    {
        float2( 0.0f,  0.65f),
        float2( 0.65f, -0.65f),
        float2(-0.65f, -0.65f)
    };

    float4 colors[3] =
    {
        float4(0.10f, 0.90f, 0.35f, 1.0f),
        float4(0.95f, 0.25f, 0.15f, 1.0f),
        float4(0.15f, 0.40f, 1.00f, 1.0f)
    };

    VSOutput output;
    output.Position = float4(positions[vertexId], 0.5f, 1.0f);
    output.Color = colors[vertexId];

    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.Color;
}
)";
    }

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

        if (!CreateDepthStencilBuffer())
        {
            Core::Logger::Fatal("DX11Device", "Failed to create depth stencil buffer.");
            Shutdown();
            return false;
        }

        if (!CreateDebugTrianglePipeline())
        {
            Core::Logger::Fatal("DX11Device", "Failed to create debug triangle pipeline.");
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

        ReleaseDebugTrianglePipeline();
        ReleaseBackBufferResources();

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
        if (!mInitialized || !mDeviceContext || !mBackBufferRenderTargetView || !mDepthStencilView)
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

        mDeviceContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView.Get());

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(mBackBufferWidth);
        viewport.Height = static_cast<float>(mBackBufferHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        mDeviceContext->RSSetViewports(1, &viewport);

        mDeviceContext->ClearRenderTargetView(mBackBufferRenderTargetView.Get(), color);
        mDeviceContext->ClearDepthStencilView(
            mDepthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0
        );
    }

    void DX11Device::DrawDebugTriangle()
    {
        if (!mInitialized || !mDeviceContext || !mDebugTriangleVertexShader || !mDebugTrianglePixelShader)
        {
            return;
        }

        mDeviceContext->IASetInputLayout(nullptr);
        mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        mDeviceContext->VSSetShader(mDebugTriangleVertexShader.Get(), nullptr, 0);
        mDeviceContext->PSSetShader(mDebugTrianglePixelShader.Get(), nullptr, 0);

        mDeviceContext->Draw(3, 0);
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

        if (mDeviceContext)
        {
            mDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
            mDeviceContext->Flush();
        }

        ReleaseBackBufferResources();

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

        if (!CreateBackBufferRenderTarget())
        {
            return false;
        }

        if (!CreateDepthStencilBuffer())
        {
            return false;
        }

        return true;
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

        const D3D_FEATURE_LEVEL requestedFeatureLevelsModern[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        const D3D_FEATURE_LEVEL requestedFeatureLevelsFallback[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        D3D_FEATURE_LEVEL createdFeatureLevel{};

        HRESULT result = ::D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            deviceFlags,
            requestedFeatureLevelsModern,
            static_cast<UINT>(sizeof(requestedFeatureLevelsModern) / sizeof(requestedFeatureLevelsModern[0])),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            mSwapChain.GetAddressOf(),
            mDevice.GetAddressOf(),
            &createdFeatureLevel,
            mDeviceContext.GetAddressOf()
        );

        if (result == E_INVALIDARG)
        {
            mSwapChain.Reset();
            mDeviceContext.Reset();
            mDevice.Reset();

            result = ::D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                deviceFlags,
                requestedFeatureLevelsFallback,
                static_cast<UINT>(sizeof(requestedFeatureLevelsFallback) / sizeof(requestedFeatureLevelsFallback[0])),
                D3D11_SDK_VERSION,
                &swapChainDesc,
                mSwapChain.GetAddressOf(),
                mDevice.GetAddressOf(),
                &createdFeatureLevel,
                mDeviceContext.GetAddressOf()
            );
        }

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

    bool DX11Device::CreateDepthStencilBuffer()
    {
        if (!mDevice)
        {
            return false;
        }

        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = static_cast<UINT>(mBackBufferWidth);
        textureDesc.Height = static_cast<UINT>(mBackBufferHeight);
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        const HRESULT textureResult = mDevice->CreateTexture2D(
            &textureDesc,
            nullptr,
            mDepthStencilTexture.GetAddressOf()
        );

        if (FAILED(textureResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("CreateTexture2D depth failed.", textureResult));
            return false;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
        viewDesc.Format = textureDesc.Format;
        viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;

        const HRESULT viewResult = mDevice->CreateDepthStencilView(
            mDepthStencilTexture.Get(),
            &viewDesc,
            mDepthStencilView.GetAddressOf()
        );

        if (FAILED(viewResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("CreateDepthStencilView failed.", viewResult));
            return false;
        }

        Core::Logger::Info("DX11Device", "Depth stencil buffer created.");

        return true;
    }

    bool DX11Device::CreateDebugTrianglePipeline()
    {
        if (!mDevice)
        {
            return false;
        }

        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(GAME_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG;
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    #else
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

        Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        HRESULT vertexCompileResult = ::D3DCompile(
            DebugTriangleShaderSource,
            std::strlen(DebugTriangleShaderSource),
            "DebugTriangle",
            nullptr,
            nullptr,
            "VSMain",
            "vs_4_0",
            compileFlags,
            0,
            vertexShaderBlob.GetAddressOf(),
            errorBlob.GetAddressOf()
        );

        if (FAILED(vertexCompileResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("Debug triangle vertex shader compile failed.", vertexCompileResult));
            Core::Logger::Error("DX11Device", BlobToString(errorBlob.Get()));
            return false;
        }

        errorBlob.Reset();

        HRESULT pixelCompileResult = ::D3DCompile(
            DebugTriangleShaderSource,
            std::strlen(DebugTriangleShaderSource),
            "DebugTriangle",
            nullptr,
            nullptr,
            "PSMain",
            "ps_4_0",
            compileFlags,
            0,
            pixelShaderBlob.GetAddressOf(),
            errorBlob.GetAddressOf()
        );

        if (FAILED(pixelCompileResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("Debug triangle pixel shader compile failed.", pixelCompileResult));
            Core::Logger::Error("DX11Device", BlobToString(errorBlob.Get()));
            return false;
        }

        const HRESULT vertexShaderResult = mDevice->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            mDebugTriangleVertexShader.GetAddressOf()
        );

        if (FAILED(vertexShaderResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("CreateVertexShader failed.", vertexShaderResult));
            return false;
        }

        const HRESULT pixelShaderResult = mDevice->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            mDebugTrianglePixelShader.GetAddressOf()
        );

        if (FAILED(pixelShaderResult))
        {
            Core::Logger::Error("DX11Device", FormatHRESULT("CreatePixelShader failed.", pixelShaderResult));
            return false;
        }

        Core::Logger::Info("DX11Device", "Debug triangle pipeline created.");

        return true;
    }

    void DX11Device::ReleaseBackBufferResources()
    {
        mDepthStencilView.Reset();
        mDepthStencilTexture.Reset();
        mBackBufferRenderTargetView.Reset();
    }

    void DX11Device::ReleaseDebugTrianglePipeline()
    {
        mDebugTrianglePixelShader.Reset();
        mDebugTriangleVertexShader.Reset();
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

    Core::String DX11Device::BlobToString(ID3DBlob* blob)
    {
        if (blob == nullptr || blob->GetBufferPointer() == nullptr || blob->GetBufferSize() == 0)
        {
            return "No compiler error details.";
        }

        const char* text = static_cast<const char*>(blob->GetBufferPointer());
        return Core::String(text, text + blob->GetBufferSize());
    }
}