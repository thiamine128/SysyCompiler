//
// Created by slty5 on 24-9-20.
//

#include "Lexer.h"

#include <iostream>

#include "../error/CompilerException.h"

namespace thm {
    void Lexer::next(Token &token) {
        char ch;
        std::string content;
        ch = input_.get();
        while (isspace(ch)) {
            if (ch == '\n') {
                currentLine += 1;
            }
            ch = input_.get();
        }
        if (isalpha(ch) || ch == '_') {
            content += ch;
            ch = input_.get();
            while (isalpha(ch) || isdigit(ch) || ch == '_') {
                content += ch;
                ch = input_.get();
            }
            input_.unget();

            token.type = reserve(content);
            token.content = content;
            token.lineno = currentLine;
        } else if (isdigit(ch)) {
            content += ch;
            ch = input_.get();
            while (isdigit(ch)) {
                content += ch;
                ch = input_.get();
            }
            input_.unget();

            token.type = INTCON;
            token.content = content;
            token.lineno = currentLine;
        } else if (ch == '\'') {
            content += ch;
            ch = input_.get();
            if (ch == '\\') {
                content += ch;
                ch = input_.get();
            }
            content += ch;
            ch = input_.get();
            if (ch == '\'') {
                content += ch;
            }
            token.content = content;
            token.type = CHRCON;
            token.lineno = currentLine;
        } else if (ch == '\"') {
            content += ch;
            ch = input_.get();
            while (ch != '\"') {
                content += ch;
                ch = input_.get();
                if (ch == '\\') {
                    content += ch;
                    ch = input_.get();
                    content += ch;
                    ch = input_.get();
                }
            }
            content += ch;
            token.content = content;
            token.type = STRCON;
            token.lineno = currentLine;
        } else if (ch == '/') {
            content += ch;
            ch = input_.get();
            if (ch == '*') {
                for (;;) {
                    do {
                        ch = input_.get();
                        if (ch == '\n') {
                            currentLine += 1;
                        }
                    } while (ch != '*');
                    do {
                        ch = input_.get();
                        if (ch == '\n') {
                            currentLine += 1;
                        } else if (ch == '/') {
                            next(token);
                            return;
                        }
                    } while (ch == '*');
                }
            }
            if (ch == '/') {
                do {
                    ch = input_.get();
                } while (ch != '\n' && ch != EOF);
                currentLine += 1;
                if (ch == EOF) {
                    content += ch;
                    token.content = content;
                    token.lineno = currentLine;
                    token.type = TK_EOF;
                    return;
                }
                next(token);
                return;
            }
            input_.unget();
            token.content = content;
            token.type = DIV;
            token.lineno = currentLine;
        } else if (ch == '&') {
            content += ch;
            ch = input_.get();
            if (ch != '&') {
                input_.unget();
                token.content = content;
                token.type = AND;
                token.lineno = currentLine;
                throw CompilerException(SYMBOL_ERROR, currentLine);
            }
            content += ch;
            token.content = content;
            token.type = AND;
            token.lineno = currentLine;
        } else if (ch == '|') {
            content += ch;
            ch = input_.get();
            if (ch != '|') {
                input_.unget();
                token.content = content;
                token.type = OR;
                token.lineno = currentLine;
                throw CompilerException(SYMBOL_ERROR, currentLine);;
            }
            content += ch;
            token.content = content;
            token.type = OR;
            token.lineno = currentLine;
        } else if (ch == '>') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = GEQ;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = GRE;
                token.lineno = currentLine;
            }
        } else if (ch == '<') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = LEQ;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = LSS;
                token.lineno = currentLine;
            }
        } else if (ch == '!') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = NEQ;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = NOT;
                token.lineno = currentLine;
            }
        } else if (ch == '=') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = EQL;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = ASSIGN;
                token.lineno = currentLine;
            }
        } else if (ch == '+') {
            content += ch;
            token.content = content;
            token.type = PLUS;
            token.lineno = currentLine;
        } else if (ch == '-') {
            content += ch;
            token.content = content;
            token.type = MINU;
            token.lineno = currentLine;
        } else if (ch == '*') {
            content += ch;
            token.content = content;
            token.type = MULT;
            token.lineno = currentLine;
        } else if (ch == '%') {
            content += ch;
            token.content = content;
            token.type = MOD;
            token.lineno = currentLine;
        } else if (ch == ';') {
            content += ch;
            token.content = content;
            token.type = SEMICN;
            token.lineno = currentLine;
        } else if (ch == ',') {
            content += ch;
            token.content = content;
            token.type = COMMA;
            token.lineno = currentLine;
        } else if (ch == '(') {
            content += ch;
            token.content = content;
            token.type = LPARENT;
            token.lineno = currentLine;
        } else if (ch == ')') {
            content += ch;
            token.content = content;
            token.type = RPARENT;
            token.lineno = currentLine;
        } else if (ch == '{') {
            content += ch;
            token.content = content;
            token.type = LBRACE;
            token.lineno = currentLine;
        } else if (ch == '}') {
            content += ch;
            token.content = content;
            token.type = RBRACE;
            token.lineno = currentLine;
        } else if (ch == '[') {
            content += ch;
            token.content = content;
            token.type = LBRACK;
            token.lineno = currentLine;
        } else if (ch == ']') {
            content += ch;
            token.content = content;
            token.type = RBRACK;
            token.lineno = currentLine;
        } else if (ch == EOF) {
            content += ch;
            token.content = content;
            token.type = TK_EOF;
            token.lineno = currentLine;
        } else {
            throw CompilerException(ILLEGAL_CHARACTER, currentLine);
        }

    }

} // thm