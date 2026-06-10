#pragma once

#include <cstdint>

namespace Engine
{
    struct FApplicationDesc
    {
        const char* ApplicationName = "Application";
        const char* LogDirectory = "Logs/Application";

        const wchar_t* Title = L"Application";

        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;

        bool EnableLogging = true;
        bool LogToConsole = true;
        bool LogToFile = true;

        bool EnableDebugRenderer = true;
        bool EnableVSync = true;

        bool EnableFrameStatsTitle = true;
        double FrameStatsTitleUpdateIntervalSeconds = 0.5;

        bool EnableFrameLimit = false;
        double TargetFrameRate = 144.0;
    };

    int RunWindowApplication(const FApplicationDesc& desc);

    int RunWindowApplication(
        const wchar_t* title,
        std::uint32_t width,
        std::uint32_t height
    );
}