#include "Core/Logger.h"

int main(int argc, char* argv[])
{
    (void)argv;

    Core::LoggerConfig loggerConfig;
    loggerConfig.ApplicationName = "GameClient";
    loggerConfig.LogDirectory = "Logs/GameClient";

    Core::Logger::Initialize(loggerConfig);

    Core::Logger::Info("GameClient", "Application started");
    Core::Logger::Info("GameClient", "Argument count: " + std::to_string(argc));

    Core::Logger::Info("GameClient", "Application finished");

    Core::Logger::Shutdown();

    return 0;
}