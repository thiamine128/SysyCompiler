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

            token.type = Token::INTCON;
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
            token.type = Token::CHRCON;
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
            token.type = Token::STRCON;
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
                    token.type = Token::TK_EOF;
                    return;
                }
                next(token);
                return;
            }
            input_.unget();
            token.content = content;
            token.type = Token::DIV;
            token.lineno = currentLine;
        } else if (ch == '&') {
            content += ch;
            ch = input_.get();
            if (ch != '&') {
                input_.unget();
                token.content = content;
                token.type = Token::AND;
                token.lineno = currentLine;
                errorReporter_.error(CompilerException(ILLEGAL_TOKEN, currentLine));
                return;
            }
            content += ch;
            token.content = content;
            token.type = Token::AND;
            token.lineno = currentLine;
        } else if (ch == '|') {
            content += ch;
            ch = input_.get();
            if (ch != '|') {
                input_.unget();
                token.content = content;
                token.type = Token::OR;
                token.lineno = currentLine;
                errorReporter_.error(CompilerException(ILLEGAL_TOKEN, currentLine));
                return;
            }
            content += ch;
            token.content = content;
            token.type = Token::OR;
            token.lineno = currentLine;
        } else if (ch == '>') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = Token::GEQ;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = Token::GRE;
                token.lineno = currentLine;
            }
        } else if (ch == '<') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = Token::LEQ;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = Token::LSS;
                token.lineno = currentLine;
            }
        } else if (ch == '!') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = Token::NEQ;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = Token::NOT;
                token.lineno = currentLine;
            }
        } else if (ch == '=') {
            content += ch;
            ch = input_.get();
            if (ch == '=') {
                content += ch;
                token.content = content;
                token.type = Token::EQL;
                token.lineno = currentLine;
            } else {
                input_.unget();
                token.content = content;
                token.type = Token::ASSIGN;
                token.lineno = currentLine;
            }
        } else if (ch == '+') {
            content += ch;
            token.content = content;
            token.type = Token::PLUS;
            token.lineno = currentLine;
        } else if (ch == '-') {
            content += ch;
            token.content = content;
            token.type = Token::MINU;
            token.lineno = currentLine;
        } else if (ch == '*') {
            content += ch;
            token.content = content;
            token.type = Token::MULT;
            token.lineno = currentLine;
        } else if (ch == '%') {
            content += ch;
            token.content = content;
            token.type = Token::MOD;
            token.lineno = currentLine;
        } else if (ch == ';') {
            content += ch;
            token.content = content;
            token.type = Token::SEMICN;
            token.lineno = currentLine;
        } else if (ch == ',') {
            content += ch;
            token.content = content;
            token.type = Token::COMMA;
            token.lineno = currentLine;
        } else if (ch == '(') {
            content += ch;
            token.content = content;
            token.type = Token::LPARENT;
            token.lineno = currentLine;
        } else if (ch == ')') {
            content += ch;
            token.content = content;
            token.type = Token::RPARENT;
            token.lineno = currentLine;
        } else if (ch == '{') {
            content += ch;
            token.content = content;
            token.type = Token::LBRACE;
            token.lineno = currentLine;
        } else if (ch == '}') {
            content += ch;
            token.content = content;
            token.type = Token::RBRACE;
            token.lineno = currentLine;
        } else if (ch == '[') {
            content += ch;
            token.content = content;
            token.type = Token::LBRACK;
            token.lineno = currentLine;
        } else if (ch == ']') {
            content += ch;
            token.content = content;
            token.type = Token::RBRACK;
            token.lineno = currentLine;
        } else if (ch == EOF) {
            content += ch;
            token.content = content;
            token.type = Token::TK_EOF;
            token.lineno = currentLine;
        } else {
            errorReporter_.error(CompilerException(ILLEGAL_CHARACTER, currentLine));
        }

    }

    void Lexer::tokenize(TokenStream &tokenStream) {
        for (;;) {
            Token token;
            next(token);

            if (token.type == Token::TK_EOF) {
                break;
            }
            tokenStream.put(token);
            if (logger_) {
                logger_->stream() << token;
            }
        }
    }
} // thm