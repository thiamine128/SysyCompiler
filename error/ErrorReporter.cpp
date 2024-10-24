//
// Created by slty5 on 24-9-27.
//

#include "ErrorReporter.h"

#include <memory>
#include <ostream>
#include "../core/Logger.h"

namespace thm {

    void ErrorReporter::error(CompilerException const &error) {
        errors.push(error);
    }
    bool ErrorReporter::hasErrors() const {
        return !errors.empty();
    }
    void ErrorReporter::printErrors(std::shared_ptr<Logger> logger) {
        int prev = -1;
        while (!errors.empty()) {
            auto error = errors.top();
            errors.pop();
            int lineno = error.line;
            if (lineno != prev && getErrorCode(error.errorType) != '-')
                logger->stream() << error.line << " " << getErrorCode(error.errorType) << std::endl;
            prev = lineno;
        }
    }

} // thm