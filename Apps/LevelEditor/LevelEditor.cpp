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

    createInfo.EnableFrameLimit = true;
    createInfo.TargetFrameRate = 60;

    return Engine::RunWindowApplication(createInfo);
}