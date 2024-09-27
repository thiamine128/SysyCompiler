//
// Created by slty5 on 24-9-27.
//

#include "TokenStream.h"

#include <iostream>

namespace thm {
    TokenStream::TokenStream(std::vector<Token> &tokens) : tokens_(tokens) {
        offset = -1;
    }

    Token const& TokenStream::next() {
        if (offset + 1 < tokens_.size()) {
            offset++;
        }
        return tokens_[offset];
    }

    Token const& TokenStream::unget() {
        if (offset >= 0)
            --offset;
        return tokens_[offset < 0 ? 0 : offset];
    }
} // thm