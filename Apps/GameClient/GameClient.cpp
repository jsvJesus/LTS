#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Engine::EngineCreateInfo createInfo;
    createInfo.ApplicationName = "GameClient";
    createInfo.LogDirectory = "Logs/GameClient";

    createInfo.MainWindow.Title = "Game Client";
    createInfo.MainWindow.Width = 1280;
    createInfo.MainWindow.Height = 720;
    createInfo.MainWindow.StartCentered = true;
    createInfo.MainWindow.Resizable = true;
    createInfo.MainWindow.VisibleOnCreate = false;

    createInfo.EnableFrameLimit = true;
    createInfo.TargetFrameRate = 60;

    return Engine::RunWindowApplication(createInfo);
}