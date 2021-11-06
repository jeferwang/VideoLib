#pragma once

#include "Logger.h"

namespace VP {
    namespace Logger {
        class VP_DLL_EXPORT ConsoleLogWriter : public ILogWriter {
        public:
            void Write(LogLevel level, const char *time, const char *message) override;
        };
    }
}
