//
// Created by slty5 on 24-9-27.
//

#include "AbstractSyntaxTree.h"

namespace thm {
    void ASTNode::consume(std::vector<Token> &tokens) {
        this->tokens = tokens;
        tokens.clear();
    }

    std::ostream & operator<<(std::ostream &os, const ASTNode &node) {
        if (node.nodeType() != ASTNode::BLOCKITEM && node.nodeType() != ASTNode::DECL && node.nodeType() != ASTNode::BTYPE)
            os << "<" << nodeTypeToString(node.nodeType()) << ">" << std::endl;
        return os;
    }

    std::string nodeTypeToString(ASTNode::ASTNodeType type) {
        switch (type) {
#define X(a, b, c) \
        case ASTNode::c: \
        return b;
            ASTNODES
            default:
                return "UNKNOWN";
        }
#undef X
    }
} // thm