//
// Created by jefer on 2021/9/30.
//

#include <cstdio>
#include "ConsoleLogWriter.h"

namespace XLog {
    void ConsoleLogWriter::Write(
            XLog::LogLevel level,
            const char *time,
            const char *message) {
        std::printf("%s %s %s\n", time, Logger::GetLogLevelName(level), message);
    }
}
