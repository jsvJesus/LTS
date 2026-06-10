#pragma once

#include "Core/BaseTypes.h"
#include "Platform/Window.h"

namespace Engine
{
    struct EngineCreateInfo final
    {
        Core::String ApplicationName = "Application";
        Core::Path LogDirectory = "Logs/Application";

        Platform::WindowCreateInfo MainWindow;

        bool EnableFrameLimit = true;
        Core::u32 TargetFrameRate = 60;
    };

    int RunWindowApplication(const EngineCreateInfo& createInfo);
}