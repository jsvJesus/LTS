#pragma once

#include <cstdint>

#include "Core/Math/Math.h"

namespace Render
{
    enum class ERenderBackend : std::uint8_t
    {
        DX11 = 0
    };

    struct FRenderColor
    {
        float R = 0.02f;
        float G = 0.02f;
        float B = 0.025f;
        float A = 1.0f;
    };

    struct FRenderViewport
    {
        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;
    };

    struct FRenderViewInfo
    {
        Core::Matrix4 ViewMatrix = Core::Matrix4::Identity();
        Core::Matrix4 ProjectionMatrix = Core::Matrix4::Identity();
        Core::Matrix4 ViewProjectionMatrix = Core::Matrix4::Identity();

        Core::Vector3 CameraPosition = Core::Vector3::Zero();
    };

    struct FRenderSystemDesc
    {
        void* NativeWindowHandle = nullptr;

        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;

        ERenderBackend Backend = ERenderBackend::DX11;

        bool EnableDebugLayer = false;
        bool EnableDebugRenderer = true;
        bool EnableDebugRendering = true;
        bool EnableVSync = true;

        FRenderColor ClearColor {};
    };

    struct FRenderFrameInfo
    {
        double DeltaSeconds = 0.0;
        std::uint64_t FrameIndex = 0;

        bool HasViewInfo = false;
        FRenderViewInfo ViewInfo {};
    };
}