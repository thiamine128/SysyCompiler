//
// Created by slty5 on 24-9-24.
//

#include "CompilerException.h"

namespace thm {
    char getErrorCode(ErrorType errorType) {
        switch (errorType) {
#define X(a, b) \
    case ErrorType::a: \
        return b;
            ERROR_TYPES
            default:
                return ' ';
        }
#undef X
    }

} // thm