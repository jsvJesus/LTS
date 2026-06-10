#include "Core/Logger.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Core
{
    namespace
    {
        std::mutex GLoggerMutex;
        LoggerConfig GLoggerConfig;
        std::ofstream GLogFile;
        bool GLoggerInitialized = false;

        std::tm GetLocalTime(const std::time_t time)
        {
            std::tm result{};

        #if defined(_WIN32)
            localtime_s(&result, &time);
        #else
            localtime_r(&time, &result);
        #endif

            return result;
        }

        std::ostream& GetConsoleStream(const LogLevel level)
        {
            switch (level)
            {
            case LogLevel::Warning:
            case LogLevel::Error:
            case LogLevel::Fatal:
                return std::cerr;

            case LogLevel::Trace:
            case LogLevel::Info:
            default:
                return std::cout;
            }
        }
    }

    bool Logger::Initialize(const LoggerConfig& config)
    {
        std::scoped_lock lock(GLoggerMutex);

        if (GLoggerInitialized)
        {
            return true;
        }

        GLoggerConfig = config;

        if (GLoggerConfig.ApplicationName.empty())
        {
            GLoggerConfig.ApplicationName = "Application";
        }

        if (GLoggerConfig.LogDirectory.empty())
        {
            GLoggerConfig.LogDirectory = "Logs";
        }

        if (GLoggerConfig.WriteToFile)
        {
            std::error_code errorCode;
            std::filesystem::create_directories(GLoggerConfig.LogDirectory, errorCode);

            if (errorCode)
            {
                std::cerr << "[Logger] Failed to create log directory: "
                          << GLoggerConfig.LogDirectory.string()
                          << " | Error: "
                          << errorCode.message()
                          << '\n';

                GLoggerConfig.WriteToFile = false;
            }
            else
            {
                const String safeName = SanitizeFileName(GLoggerConfig.ApplicationName);
                const Path logFilePath = GLoggerConfig.LogDirectory / (safeName + "_" + GetTimestampForFileName() + ".log");

                GLogFile.open(logFilePath, std::ios::out | std::ios::app);

                if (!GLogFile.is_open())
                {
                    std::cerr << "[Logger] Failed to open log file: "
                              << logFilePath.string()
                              << '\n';

                    GLoggerConfig.WriteToFile = false;
                }
            }
        }

        GLoggerInitialized = true;

        Write(LogLevel::Info, "Logger", "Logger initialized");
        Write(LogLevel::Info, "Core", String("Build configuration: ") + String(GetBuildConfigurationName()));

        return true;
    }

    void Logger::Shutdown()
    {
        std::scoped_lock lock(GLoggerMutex);

        if (!GLoggerInitialized)
        {
            return;
        }

        const String line = BuildLogLine(LogLevel::Info, "Logger", "Logger shutdown");

        if (GLoggerConfig.WriteToConsole)
        {
            std::cout << line << '\n';
        }

        if (GLoggerConfig.WriteToFile && GLogFile.is_open())
        {
            GLogFile << line << '\n';
            GLogFile.flush();
            GLogFile.close();
        }

        GLoggerInitialized = false;
    }

    bool Logger::IsInitialized()
    {
        std::scoped_lock lock(GLoggerMutex);
        return GLoggerInitialized;
    }

    void Logger::Write(const LogLevel level, const StringView category, const StringView message)
    {
        std::scoped_lock lock(GLoggerMutex);

        if (!GLoggerInitialized)
        {
            return;
        }

        const String line = BuildLogLine(level, category, message);

        if (GLoggerConfig.WriteToConsole)
        {
            std::ostream& stream = GetConsoleStream(level);
            stream << line << '\n';

            if (GLoggerConfig.FlushEachMessage)
            {
                stream.flush();
            }
        }

        if (GLoggerConfig.WriteToFile && GLogFile.is_open())
        {
            GLogFile << line << '\n';

            if (GLoggerConfig.FlushEachMessage)
            {
                GLogFile.flush();
            }
        }
    }

    void Logger::Trace(const StringView category, const StringView message)
    {
        Write(LogLevel::Trace, category, message);
    }

    void Logger::Info(const StringView category, const StringView message)
    {
        Write(LogLevel::Info, category, message);
    }

    void Logger::Warning(const StringView category, const StringView message)
    {
        Write(LogLevel::Warning, category, message);
    }

    void Logger::Error(const StringView category, const StringView message)
    {
        Write(LogLevel::Error, category, message);
    }

    void Logger::Fatal(const StringView category, const StringView message)
    {
        Write(LogLevel::Fatal, category, message);
    }

    String Logger::BuildLogLine(const LogLevel level, const StringView category, const StringView message)
    {
        std::ostringstream stream;

        stream << '[' << GetTimestampForLine() << ']'
               << '[' << GetLevelName(level) << ']'
               << '[' << category << "] "
               << message;

        return stream.str();
    }

    String Logger::GetTimestampForLine()
    {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        const std::tm localTime = GetLocalTime(time);

        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count() % 1000;

        std::ostringstream stream;

        stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
               << '.'
               << std::setw(3)
               << std::setfill('0')
               << milliseconds;

        return stream.str();
    }

    String Logger::GetTimestampForFileName()
    {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        const std::tm localTime = GetLocalTime(time);

        std::ostringstream stream;
        stream << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");

        return stream.str();
    }

    StringView Logger::GetLevelName(const LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Trace:
            return "Trace";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Fatal:
            return "Fatal";
        default:
            return "Unknown";
        }
    }

    String Logger::SanitizeFileName(const StringView value)
    {
        String result(value);

        std::replace_if(
            result.begin(),
            result.end(),
            [](const char character)
            {
                switch (character)
                {
                case '<':
                case '>':
                case ':':
                case '"':
                case '/':
                case '\\':
                case '|':
                case '?':
                case '*':
                case ' ':
                    return true;
                default:
                    return false;
                }
            },
            '_'
        );

        if (result.empty())
        {
            result = "Application";
        }

        return result;
    }
}