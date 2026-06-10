#include "Engine.h"

#include "EngineLoop.h"

namespace Engine
{
    int RunWindowApplication(const FApplicationDesc& desc)
    {
        EngineLoop loop;

        if (!loop.Initialize(desc))
            return -1;

        const int result = loop.Run();

        loop.Shutdown();

        return result;
    }

    int RunWindowApplication(
        const wchar_t* title,
        std::uint32_t width,
        std::uint32_t height
    )
    {
        FApplicationDesc desc {};
        desc.Title = title;
        desc.Width = width;
        desc.Height = height;

        return RunWindowApplication(desc);
    }
}