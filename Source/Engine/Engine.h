#pragma once

#include "Core/BaseTypes.h"
#include "Platform/Window.h"
#include "Render/DX11/DX11Device.h"

namespace Engine
{
    struct EngineCreateInfo final
    {
        Core::String ApplicationName = "Application";
        Core::Path LogDirectory = "Logs/Application";

        Platform::WindowCreateInfo MainWindow;

        bool EnableRendering = true;
        bool EnableFrameLimit = true;

        bool EnableVSync = true;

        Core::u32 TargetFrameRate = 60;

        Render::ClearColor ClearColor;
    };

    int RunWindowApplication(const EngineCreateInfo& createInfo);
}