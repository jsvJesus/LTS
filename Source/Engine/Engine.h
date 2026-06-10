#pragma once

#include <cstdint>

namespace Engine
{
    struct FApplicationDesc
    {
        const wchar_t* Title = L"Application";

        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;

        bool EnableDebugRenderer = true;
        bool EnableVSync = true;
    };

    int RunWindowApplication(const FApplicationDesc& desc);

    int RunWindowApplication(
        const wchar_t* title,
        std::uint32_t width,
        std::uint32_t height
    );
}