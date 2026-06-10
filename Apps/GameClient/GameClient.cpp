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
    appDesc.EnableVSync = true;

    appDesc.EnableFrameStatsTitle = true;
    appDesc.FrameStatsTitleUpdateIntervalSeconds = 0.5;

    return Engine::RunWindowApplication(appDesc);
}