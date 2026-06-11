#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Engine::FApplicationDesc appDesc {};
    appDesc.ApplicationName = "LevelEditor";
    appDesc.LogDirectory = "Logs/LevelEditor";

    appDesc.Title = L"Level Editor";
    appDesc.Width = 1600;
    appDesc.Height = 900;

    appDesc.EnableLogging = true;
    appDesc.LogToConsole = true;
    appDesc.LogToFile = true;

    appDesc.EnableDebugRenderer = true;
    appDesc.EnableDebugRendering = true;
    appDesc.EnableVSync = true;

    appDesc.ClearColor.R = 0.030f;
    appDesc.ClearColor.G = 0.032f;
    appDesc.ClearColor.B = 0.036f;
    appDesc.ClearColor.A = 1.0f;

    appDesc.EnableFrameStatsTitle = true;
    appDesc.FrameStatsTitleUpdateIntervalSeconds = 0.5;

    appDesc.EnableFrameLimit = true;
    appDesc.TargetFrameRate = 144.0;

    return Engine::RunWindowApplication(appDesc);
}