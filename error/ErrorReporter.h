//
// Created by slty5 on 24-9-27.
//

#ifndef ERRORREPORTER_H
#define ERRORREPORTER_H
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "CompilerException.h"

namespace thm {
    class Logger;

    class ErrorReporter {
    private:
        std::priority_queue<CompilerException> errors;
    public:
        void error(CompilerException const& error);
        std::priority_queue<CompilerException> const& getErrors() const;
        bool hasErrors() const;
        void printErrors(std::shared_ptr<Logger> logger);
    };

} // thm

#endif //ERRORREPORTER_H
