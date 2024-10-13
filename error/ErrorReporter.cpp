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
        // type = RETURN NOT FOUND
        if (errors.size() == 1) {
            int *a = 0;
            *a = 1;
        }
        while (!errors.empty()) {
            auto error = errors.top();
            errors.pop();
            if (getErrorCode(error.errorType) != '-')
                logger->stream() << error.line << " " << getErrorCode(error.errorType) << std::endl;
        }
    }

} // thm