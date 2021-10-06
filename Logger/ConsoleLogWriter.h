#pragma once

#include "Writer.h"

namespace VP {
    namespace Logger {
        class DllExport ConsoleLogWriter : public ILogWriter {
        public:
            void Write(LogLevel level, const char *time, const char *message) override;
        };
    }
}
