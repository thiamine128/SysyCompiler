//
// Created by slty5 on 24-9-27.
//

#include "AbstractSyntaxTree.h"

namespace thm {
    void ASTNode::consume(std::vector<Token> &tokens) {
        this->tokens = tokens;
        tokens.clear();
    }
} // thm