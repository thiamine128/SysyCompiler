//
// Created by slty5 on 24-9-28.
//

#include "Logger.h"

namespace thm {
    Logger::Logger(std::string const &logFile) {
        this->logFile.open(logFile);
    }

    Logger::~Logger() {
        logFile.close();
    }
} // thm