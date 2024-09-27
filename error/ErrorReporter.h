//
// Created by slty5 on 24-9-27.
//

#ifndef ERRORREPORTER_H
#define ERRORREPORTER_H
#include <string>
#include <vector>

#include "CompilerException.h"

namespace thm {

    class ErrorReporter {
    private:
        std::vector<CompilerException> errors;
    public:
        void error(CompilerException const& error);
        std::vector<CompilerException> const& getErrors() const;
        bool hasErrors() const;
        void printErrors(std::ostream& out) const;
    };

} // thm

#endif //ERRORREPORTER_H
