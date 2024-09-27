//
// Created by slty5 on 24-9-27.
//

#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H
#include <memory>
#include <vector>

#include "Token.h"

namespace thm {

    class TokenStream {
    private:
        std::vector<Token>& tokens_;
        int offset;
    public:
        TokenStream(std::vector<Token>& tokens);

        Token const& next();
        Token const& unget();
    };

} // thm

#endif //TOKENSTREAM_H
