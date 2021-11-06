#include <vector>
#include <ctime>
#include <cstdarg>
#include "vadefs.h"
#include "Logger.h"

namespace VP {
    namespace Logger {
        std::set<ILogWriter *> Log::Writers;

        void Log::RegisterLogger(ILogWriter *InWriter) {
            Writers.emplace(InWriter);
        }

        void Log::UnregisterLogger(ILogWriter *InWriter) {
            Writers.erase(InWriter);
        }

        void Log::Write(LogLevel Level, const char *Format, ...) {
            // Time
            char Time[30] = {'\0'};
            GetTime(Time, 30);

            // Message
            va_list va;
                    va_start(va, Format);
            char Msg[2048] = {'\0'};
            vsprintf_s(Msg, sizeof(Msg), Format, va);
                    va_end(va);

            // Print
            for (const auto Writer: Writers) {
                Writer->Write(Level, Time, Msg);
            }
        }

        const char *Log::GetLogLevelName(LogLevel level) {
            static const char *s[] = {
                    "DEBUG",
                    "INFO",
                    "WARNING",
                    "ERROR",
                    "FATAL"
            };
            return s[(int) level];
        }

        void Log::GetTime(char *InBuffer, int32_t InBufferLen) {
            auto Time = std::time(nullptr);
            std::tm Tm{};
            localtime_s(&Tm, &Time);
            strftime(InBuffer, InBufferLen, "%Y/%m/%d %H:%M:%S", &Tm);
        }
    }
}
