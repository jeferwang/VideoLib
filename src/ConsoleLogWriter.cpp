//
// Created by jefer on 2021/9/30.
//

#include <cstdio>
#include "ConsoleLogWriter.h"

namespace VP {
    namespace Logger {
        void ConsoleLogWriter::Write(
                VP::Logger::LogLevel level,
                const char *time,
                const char *message) {
            std::printf("%s %s %s\n", time, Log::GetLogLevelName(level), message);
        }
    }
}
