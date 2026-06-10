#include "Core/Logger.h"
#include "Platform/Window.h"

#include <chrono>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
    (void)argv;

    Core::LoggerConfig loggerConfig;
    loggerConfig.ApplicationName = "GameClient";
    loggerConfig.LogDirectory = "Logs/GameClient";

    Core::Logger::Initialize(loggerConfig);

    Core::Logger::Info("GameClient", "Application started");
    Core::Logger::Info("GameClient", "Argument count: " + std::to_string(argc));

    Platform::WindowCreateInfo windowCreateInfo;
    windowCreateInfo.Title = "Game Client";
    windowCreateInfo.Width = 1280;
    windowCreateInfo.Height = 720;
    windowCreateInfo.StartCentered = true;
    windowCreateInfo.Resizable = true;
    windowCreateInfo.VisibleOnCreate = false;

    Platform::Window window;

    if (!window.Create(windowCreateInfo))
    {
        Core::Logger::Fatal("GameClient", "Failed to create main window.");
        Core::Logger::Shutdown();
        return 1;
    }

    window.Show();

    Core::Logger::Info("GameClient", "Main window opened.");

    while (window.PollEvents())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    Core::Logger::Info("GameClient", "Main window closed.");

    window.Destroy();

    Core::Logger::Info("GameClient", "Application finished");

    Core::Logger::Shutdown();

    return 0;
}