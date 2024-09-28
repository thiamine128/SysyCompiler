//
// Created by slty5 on 24-9-27.
//

#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H
#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include "Token.h"

namespace thm {

    class TokenStream {
    private:
        std::deque<Token> tokens_;

        static Token emptyToken_;
    public:
        void put(Token const& token);
        bool peekType(Token::TokenType expectedType) const;
        bool peekType(int offset, Token::TokenType expectedType) const;
        bool peekType(int offset, std::vector<Token::TokenType> candidates) const;
        void peekForward(std::function<bool(Token::TokenType)> visit);

        void peekForward(std::function<bool(Token const &)> visit);

        Token const& peek() const;
        Token const& peek(int offset) const;
        Token next();
    };

} // thm

#endif //TOKENSTREAM_H
