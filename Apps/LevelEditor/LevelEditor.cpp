#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Engine::EngineCreateInfo createInfo;
    createInfo.ApplicationName = "LevelEditor";
    createInfo.LogDirectory = "Logs/LevelEditor";

    createInfo.MainWindow.Title = "Level Editor";
    createInfo.MainWindow.Width = 1600;
    createInfo.MainWindow.Height = 900;
    createInfo.MainWindow.StartCentered = true;
    createInfo.MainWindow.Resizable = true;
    createInfo.MainWindow.VisibleOnCreate = false;

    createInfo.EnableRendering = true;
    createInfo.EnableVSync = true;

    createInfo.EnableFrameLimit = true;
    createInfo.TargetFrameRate = 60;

    createInfo.ClearColor.R = 0.06f;
    createInfo.ClearColor.G = 0.065f;
    createInfo.ClearColor.B = 0.07f;
    createInfo.ClearColor.A = 1.0f;

    return Engine::RunWindowApplication(createInfo);
}