//
// Created by slty5 on 24-9-28.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>

namespace thm {

class Logger {
public:
    std::ofstream logFile;
public:
    Logger(std::string const& logFile);
    ~Logger();

    std::ostream& stream() {
        return logFile;
    }
};

} // thm

#endif //LOGGER_H
