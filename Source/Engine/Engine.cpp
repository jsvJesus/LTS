#include "Engine.h"

#include "ApplicationRuntime.h"
#include "EngineLoop.h"

#include "Core/Logger.h"

namespace Engine
{
    namespace
    {
        const char* GetSafeApplicationName(const FApplicationDesc& desc)
        {
            if (desc.ApplicationName == nullptr || desc.ApplicationName[0] == '\0')
                return "Application";

            return desc.ApplicationName;
        }

        const char* GetSafeLogDirectory(const FApplicationDesc& desc)
        {
            if (desc.LogDirectory == nullptr || desc.LogDirectory[0] == '\0')
                return "Logs/Application";

            return desc.LogDirectory;
        }

        bool InitializeLogger(const FApplicationDesc& desc)
        {
            if (!desc.EnableLogging)
                return false;

            Core::LoggerConfig loggerConfig {};
            loggerConfig.ApplicationName = GetSafeApplicationName(desc);
            loggerConfig.LogDirectory = GetSafeLogDirectory(desc);
            loggerConfig.WriteToConsole = desc.LogToConsole;
            loggerConfig.WriteToFile = desc.LogToFile;
            loggerConfig.FlushEachMessage = true;

            return Core::Logger::Initialize(loggerConfig);
        }

        void ShutdownLoggerIfNeeded(const bool loggerInitialized)
        {
            if (loggerInitialized)
            {
                Core::Logger::Shutdown();
            }
        }
    }

    const char* GetApplicationModeName(const EApplicationMode mode)
    {
        switch (mode)
        {
        case EApplicationMode::GameClient:
            return "GameClient";

        case EApplicationMode::LevelEditor:
            return "LevelEditor";

        case EApplicationMode::Tool:
            return "Tool";

        case EApplicationMode::Unknown:
        default:
            return "Unknown";
        }
    }

    int RunWindowApplication(const FApplicationDesc& desc)
    {
        const bool loggerInitialized = InitializeLogger(desc);

        Core::Logger::Info("Engine", "Starting window application.");
        Core::Logger::Info("Engine", GetApplicationModeName(desc.ApplicationMode));

        if (desc.Runtime)
        {
            Core::Logger::Info("Engine", desc.Runtime->GetRuntimeName());
        }
        else
        {
            Core::Logger::Info("Engine", "Application runtime: none.");
        }

        EngineLoop loop;

        if (!loop.Initialize(desc))
        {
            Core::Logger::Error("Engine", "Failed to initialize engine loop.");

            loop.Shutdown();
            ShutdownLoggerIfNeeded(loggerInitialized);

            return -1;
        }

        const int result = loop.Run();

        Core::Logger::Info("Engine", "Engine loop finished.");

        loop.Shutdown();

        Core::Logger::Info("Engine", "Window application shutdown completed.");

        ShutdownLoggerIfNeeded(loggerInitialized);

        return result;
    }

    int RunWindowApplication(
        const wchar_t* title,
        std::uint32_t width,
        std::uint32_t height
    )
    {
        FApplicationDesc desc {};
        desc.ApplicationMode = EApplicationMode::Unknown;
        desc.Runtime = nullptr;
        desc.ApplicationName = "Application";
        desc.LogDirectory = "Logs/Application";
        desc.Title = title;
        desc.Width = width;
        desc.Height = height;

        return RunWindowApplication(desc);
    }
}