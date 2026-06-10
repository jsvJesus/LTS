#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Engine::FApplicationDesc appDesc {};
    appDesc.Title = L"Level Editor";
    appDesc.Width = 1600;
    appDesc.Height = 900;
    appDesc.EnableDebugRenderer = true;
    appDesc.EnableVSync = true;

    return Engine::RunWindowApplication(appDesc);
}