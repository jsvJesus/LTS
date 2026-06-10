#include "Engine.h"

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

    int RunWindowApplication(const FApplicationDesc& desc)
    {
        const bool loggerInitialized = InitializeLogger(desc);

        Core::Logger::Info("Engine", "Starting window application.");

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
        desc.ApplicationName = "Application";
        desc.LogDirectory = "Logs/Application";
        desc.Title = title;
        desc.Width = width;
        desc.Height = height;

        return RunWindowApplication(desc);
    }
}