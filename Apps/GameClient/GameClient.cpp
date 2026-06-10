#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Engine::FApplicationDesc appDesc {};
    appDesc.Title = L"Game Client";
    appDesc.Width = 1280;
    appDesc.Height = 720;
    appDesc.EnableDebugRenderer = true;
    appDesc.EnableVSync = true;

    return Engine::RunWindowApplication(appDesc);
}