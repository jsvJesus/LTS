#include "Core/Logger.h"

int main(int argc, char* argv[])
{
    (void)argv;

    Core::LoggerConfig loggerConfig;
    loggerConfig.ApplicationName = "GameServer";
    loggerConfig.LogDirectory = "Logs/GameServer";

    Core::Logger::Initialize(loggerConfig);

    Core::Logger::Info("GameServer", "Application started");
    Core::Logger::Info("GameServer", "Argument count: " + std::to_string(argc));

    Core::Logger::Info("GameServer", "Application finished");

    Core::Logger::Shutdown();

    return 0;
}