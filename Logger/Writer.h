#pragma once

#include <set>

namespace VP {
    namespace Logger {
        enum class LogLevel {
            Debug = 0,
            Info,
            Warning,
            Error,
            Fatal,
        };

        class DllExport ILogWriter {
        public:
            virtual void Write(LogLevel level, const char *time, const char *message) = 0;

        };

        class DllExport Log {
        public:
            static void RegisterLogger(ILogWriter *InWriter);

            static void UnregisterLogger(ILogWriter *InWriter);

            static void Write(LogLevel Level, const char *Format, ...);

            static void GetTime(char *InBuffer, int32_t InBufferLen);

            static const char *GetLogLevelName(LogLevel level);

        private:
            static std::set<ILogWriter *> Writers;
        };
    }
}