//
// Created by slty5 on 24-9-27.
//

#include "ErrorReporter.h"

#include <ostream>

namespace thm {
    void ErrorReporter::error(CompilerException const &error) {
        errors.push_back(error);
    }
    bool ErrorReporter::hasErrors() const {
        return !errors.empty();
    }
    std::vector<CompilerException> const& ErrorReporter::getErrors() const {
        return errors;
    }
    void ErrorReporter::printErrors(std::ostream& out) const {
        for (auto error : errors) {
            if (getErrorCode(error.errorType) != '-')
                out << error.line << " " << getErrorCode(error.errorType) << std::endl;
        }
    }

} // thm