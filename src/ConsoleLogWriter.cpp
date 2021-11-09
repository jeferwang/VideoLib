//
// Created by jefer on 2021/9/30.
//

#include <cstdio>
#include "ConsoleLogWriter.h"

namespace Log {
    void ConsoleLogWriter::Write(
            Log::LogLevel level,
            const char *time,
            const char *message) {
        std::printf("%s %s %s\n", time, Logger::GetLogLevelName(level), message);
    }
}
