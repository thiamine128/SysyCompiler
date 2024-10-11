//
// Created by slty5 on 24-9-27.
//

#include "TokenStream.h"

#include <iostream>

namespace thm {
    Token TokenStream::emptyToken_ = Token(Token::TK_EOF, "", -1);

    void TokenStream::put(Token const &token) {
        tokens_.push_back(token);
    }

    bool TokenStream::peekType(Token::TokenType expectedType) const {
        return peekType(0, expectedType);
    }

    bool TokenStream::peekType(int offset, Token::TokenType expectedType) const {
        return peek(offset).type == expectedType;
    }

    bool TokenStream::peekType(int offset, std::vector<Token::TokenType> candidates) const {
        for (auto type : candidates) {
            if (peek(offset).type == type) {
                return true;
            }
        }
        return false;
    }

    void TokenStream::peekForward(std::function<bool(Token::TokenType)> visit) {
        int current = 0;
        while (current < tokens_.size() && visit(tokens_[current].type)) {
            current ++;
        }
    }

    bool TokenStream::empty() const {
        return tokens_.empty();
    }

    int TokenStream::size() const {
        return tokens_.size();
    }

    void TokenStream::peekForward(std::function<bool(Token const&)> visit) {
        int current = 0;
        while (current < tokens_.size() && visit(tokens_[current])) {
            current ++;
        }
    }

    Token const & TokenStream::peek() const {
        return peek(0);
    }

    Token const & TokenStream::peek(int offset) const {
        if (offset >= tokens_.size()) {
            return emptyToken_;
        }
        return tokens_[offset];
    }

    Token TokenStream::next() {
        Token token = tokens_.front();
        tokens_.pop_front();
        return token;
    }
} // thm