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
    appDesc.EnableVSync = true;

    appDesc.EnableFrameStatsTitle = true;
    appDesc.FrameStatsTitleUpdateIntervalSeconds = 0.5;

    return Engine::RunWindowApplication(appDesc);
}