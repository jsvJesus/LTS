#include "Core/Logger.h"
#include "Platform/Window.h"

#include <chrono>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
    (void)argv;

    Core::LoggerConfig loggerConfig;
    loggerConfig.ApplicationName = "LevelEditor";
    loggerConfig.LogDirectory = "Logs/LevelEditor";

    Core::Logger::Initialize(loggerConfig);

    Core::Logger::Info("LevelEditor", "Application started");
    Core::Logger::Info("LevelEditor", "Argument count: " + std::to_string(argc));

    Platform::WindowCreateInfo windowCreateInfo;
    windowCreateInfo.Title = "Level Editor";
    windowCreateInfo.Width = 1600;
    windowCreateInfo.Height = 900;
    windowCreateInfo.StartCentered = true;
    windowCreateInfo.Resizable = true;
    windowCreateInfo.VisibleOnCreate = false;

    Platform::Window window;

    if (!window.Create(windowCreateInfo))
    {
        Core::Logger::Fatal("LevelEditor", "Failed to create editor window.");
        Core::Logger::Shutdown();
        return 1;
    }

    window.Show();

    Core::Logger::Info("LevelEditor", "Editor window opened.");

    while (window.PollEvents())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    Core::Logger::Info("LevelEditor", "Editor window closed.");

    window.Destroy();

    Core::Logger::Info("LevelEditor", "Application finished");

    Core::Logger::Shutdown();

    return 0;
}