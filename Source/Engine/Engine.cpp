#include "Engine/Engine.h"

#include "Core/Logger.h"
#include "Engine/EngineLoop.h"

namespace Engine
{
    int RunWindowApplication(const EngineCreateInfo& createInfo)
    {
        Core::LoggerConfig loggerConfig;
        loggerConfig.ApplicationName = createInfo.ApplicationName;
        loggerConfig.LogDirectory = createInfo.LogDirectory;
        loggerConfig.WriteToConsole = true;
        loggerConfig.WriteToFile = true;
        loggerConfig.FlushEachMessage = true;

        Core::Logger::Initialize(loggerConfig);

        Core::Logger::Info("Engine", "Application boot started.");

        EngineLoop engineLoop;

        if (!engineLoop.Initialize(createInfo))
        {
            Core::Logger::Fatal("Engine", "Application initialization failed.");
            Core::Logger::Shutdown();
            return 1;
        }

        const int exitCode = engineLoop.Run();

        Core::Logger::Info("Engine", "Application boot finished.");

        Core::Logger::Shutdown();

        return exitCode;
    }

    void EngineModuleAnchor()
    {
    }
}