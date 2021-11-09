#include <set>
#include <ctime>
#include "LogWriter.h"

namespace Log {
    std::set<ILogWriter *> Logger::Writers;

    void Logger::RegisterLogger(ILogWriter *InWriter) {
        Writers.emplace(InWriter);
    }

    void Logger::UnregisterLogger(ILogWriter *InWriter) {
        Writers.erase(InWriter);
    }


    const char *Logger::GetLogLevelName(LogLevel level) {
        static const char *s[] = {
                "DEBUG",
                "INFO",
                "WARNING",
                "ERROR",
                "FATAL"
        };
        return s[(int) level];
    }

    void Logger::GetTime(char *InBuffer, int32_t InBufferLen) {
        auto Time = std::time(nullptr);
        std::tm Tm{};
        localtime_s(&Tm, &Time);
        strftime(InBuffer, InBufferLen, "%Y/%m/%d %H:%M:%S", &Tm);
    }

    void Logger::WriteLog(LogLevel Level, const char *Time, const char *Msg) {
        for (const auto Writer: Logger::Writers) {
            Writer->Write(Level, Time, Msg);
        }
    }
}
