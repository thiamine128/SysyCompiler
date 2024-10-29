//
// Created by slty5 on 24-10-27.
//

#include "LLVM.h"

#include <ostream>
#include <bits/locale_facets_nonio.h>

namespace thm {
    ValueType::ValueType(Type type, bool isPtr) : type(type), isPtr(isPtr) {
    }

    Value::Value() : valueType(ValueType::VOID, false) {

    }

    void Value::print(std::ostream &os) const {

    }

    LLVMType Value::type() const {
        return LLVMType::DEFAULT;
    }

    Use::Use(Value *user, Value *usee) : user(user), usee(usee) {

    }

    // void GlobalVariable::print(std::ostream &os) const {
    //     if (symbol->type.isArray) {
    //         os << "@" << symbol->ident.content << " = " << "dso_local" << " " << "global" << " [" << symbol->type.arrayLen << " x " << valueTypeToString(type) << "] ";
    //         if (symbol->hasInit) {
    //             os << "[";
    //             for (int i = 0; i < symbol->type.arrayLen; i++) {
    //                 os << valueTypeToString(type) << " " << symbol->initVals[i];
    //                 if (i < symbol->type.arrayLen - 1) {
    //                     os << ", ";
    //                 }
    //             }
    //             os << "]";
    //         } else {
    //             os << "zeroinitializer";
    //         }
    //     } else {
    //         os << "@" << symbol->ident.content << " = " << "dso_local" << " " << "global" << " " << valueTypeToString(type) << " " << symbol->initVal;
    //     }
    // }
    //
    // void Module::print(std::ostream &os) {
    //     os << "declare i32 @getint()" << std::endl
    //         << "declare i32 @getchar()" << std::endl
    //         << "declare void @putint(i32)"<< std::endl
    //         << "declare void @putch(i8)" << std::endl
    //         << "declare void @putstr(i8*)" << std::endl;
    //     for (auto var : variables) {
    //         var->print(os);
    //         os << std::endl;
    //     }
    //
    //     for (auto func : functions) {
    //         func->print(os);
    //         os << std::endl;
    //     }
    // }
} // thm
