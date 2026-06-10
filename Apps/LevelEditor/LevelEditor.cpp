#include "Core/Logger.h"

int main(int argc, char* argv[])
{
    (void)argv;

    Core::LoggerConfig loggerConfig;
    loggerConfig.ApplicationName = "LevelEditor";
    loggerConfig.LogDirectory = "Logs/LevelEditor";

    Core::Logger::Initialize(loggerConfig);

    Core::Logger::Info("LevelEditor", "Application started");
    Core::Logger::Info("LevelEditor", "Argument count: " + std::to_string(argc));

    Core::Logger::Info("LevelEditor", "Application finished");

    Core::Logger::Shutdown();

    return 0;
}