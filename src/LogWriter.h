#pragma once

#include "Base.h"
#include <set>

namespace XLog {
    enum class LogLevel {
        Debug = 0,
        Info,
        Warning,
        Error,
        Fatal,
    };

    class LOG_API ILogWriter {
    public:
        virtual ~ILogWriter() = default;

        virtual void Write(LogLevel level, const char *time, const char *message) = 0;
    };

    class LOG_API Logger {
    public:
        template<class ... Args>
        static void Debug(const char *Format, Args ... InArgs) {
            Logger::Write(LogLevel::Debug, Format, InArgs...);
        }

        template<class ... Args>
        static void Info(const char *Format, Args ... InArgs) {
            Logger::Write(LogLevel::Info, Format, InArgs...);
        }

        template<class ... Args>
        static void Warning(const char *Format, Args ... InArgs) {
            Logger::Write(LogLevel::Warning, Format, InArgs...);
        }

        template<class ... Args>
        static void Error(const char *Format, Args ... InArgs) {
            Logger::Write(LogLevel::Error, Format, InArgs...);
        }

        template<class ... Args>
        static void Fatal(const char *Format, Args ... InArgs) {
            Logger::Write(LogLevel::Fatal, Format, InArgs...);
        }

        template<class ... Args>
        static void Write(LogLevel Level, const char *Format, Args ... InArgs) {
            char Time[30] = {'\0'};
            GetTime(Time, 30);

            char Msg[2048] = {'\0'};
            sprintf_s(Msg, sizeof(Msg), Format, InArgs...);

            WriteLog(Level, Time, Msg);
        }

        static void RegisterLogger(ILogWriter *InWriter);

        static void UnregisterLogger(ILogWriter *InWriter);

        static void GetTime(char *InBuffer, int32_t InBufferLen);

        static const char *GetLogLevelName(LogLevel level);

    private:
        static void WriteLog(LogLevel Level, const char *Time, const char *Msg);

    private:
        static std::set<ILogWriter *> Writers;
    };
}

// static void Debug(const char *Format, ...);
// static void Info(const char *Format, ...);
// static void Warning(const char *Format, ...);
// static void Error(const char *Format, ...);
// static void Fatal(const char *Format, ...);