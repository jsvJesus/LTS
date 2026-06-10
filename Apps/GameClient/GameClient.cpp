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

    createInfo.EnableRendering = true;
    createInfo.EnableVSync = true;

    createInfo.EnableFrameLimit = true;
    createInfo.TargetFrameRate = 60;

    createInfo.ClearColor.R = 0.035f;
    createInfo.ClearColor.G = 0.045f;
    createInfo.ClearColor.B = 0.055f;
    createInfo.ClearColor.A = 1.0f;

    return Engine::RunWindowApplication(createInfo);
}