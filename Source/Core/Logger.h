#pragma once

#include "Core/BaseTypes.h"

#include <fstream>
#include <mutex>

namespace Core
{
    enum class LogLevel : u8
    {
        Trace,
        Info,
        Warning,
        Error,
        Fatal
    };

    struct LoggerConfig final
    {
        String ApplicationName = "Application";
        Path LogDirectory = "Logs";

        bool WriteToConsole = true;
        bool WriteToFile = true;
        bool FlushEachMessage = true;
    };

    class Logger final
    {
    public:
        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;

        Logger& operator=(const Logger&) = delete;
        Logger& operator=(Logger&&) = delete;

        static bool Initialize(const LoggerConfig& config);
        static void Shutdown();

        [[nodiscard]] static bool IsInitialized();

        static void Write(LogLevel level, StringView category, StringView message);

        static void Trace(StringView category, StringView message);
        static void Info(StringView category, StringView message);
        static void Warning(StringView category, StringView message);
        static void Error(StringView category, StringView message);
        static void Fatal(StringView category, StringView message);

    private:
        static String BuildLogLine(LogLevel level, StringView category, StringView message);
        static String GetTimestampForLine();
        static String GetTimestampForFileName();
        static StringView GetLevelName(LogLevel level);
        static String SanitizeFileName(StringView value);
    };
}