//
// Created by slty5 on 24-9-24.
//

#ifndef COMPILEREXCEPTION_H
#define COMPILEREXCEPTION_H
#include <exception>

#define ERROR_TYPES \
    X(ILLEGAL_CHARACTER, '-') \
    X(ILLEGAL_TOKEN, 'a') \
    X(UNEXPECTED_TOKEN, '-') \
    X(MISSING_SEMICOLON, 'i') \
    X(MISSING_RPARENT, 'j') \
    X(MISSING_RBRACK, 'k')

namespace thm {

    enum ErrorType {
    #define X(a, b) a,
        ERROR_TYPES
    #undef X
    };

    class CompilerException : public std::exception {
    public:
        ErrorType errorType;
        int line;

        CompilerException(ErrorType errorType, int line) : errorType(errorType), line(line) {}
        bool operator>(const CompilerException &other) const {
            return line > other.line;
        }
    };

    char getErrorCode(ErrorType errorType);

} // thm

#endif //COMPILEREXCEPTION_H
