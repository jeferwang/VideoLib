#pragma once

#include "LogWriter.h"

namespace Log {
    class LOG_API ConsoleLogWriter : public ILogWriter {
    public:
        void Write(LogLevel level, const char *time, const char *message) override;
    };
}
