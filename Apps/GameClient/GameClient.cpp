#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Engine::FApplicationDesc appDesc {};
    appDesc.ApplicationName = "GameClient";
    appDesc.LogDirectory = "Logs/GameClient";

    appDesc.Title = L"Game Client";
    appDesc.Width = 1280;
    appDesc.Height = 720;

    appDesc.EnableLogging = true;
    appDesc.LogToConsole = true;
    appDesc.LogToFile = true;

    appDesc.EnableDebugRenderer = true;
    appDesc.EnableDebugRendering = true;
    appDesc.EnableVSync = true;

    appDesc.ClearColor.R = 0.012f;
    appDesc.ClearColor.G = 0.014f;
    appDesc.ClearColor.B = 0.020f;
    appDesc.ClearColor.A = 1.0f;

    appDesc.EnableFrameStatsTitle = true;
    appDesc.FrameStatsTitleUpdateIntervalSeconds = 0.5;

    appDesc.EnableFrameLimit = false;
    appDesc.TargetFrameRate = 144.0;

    return Engine::RunWindowApplication(appDesc);
}