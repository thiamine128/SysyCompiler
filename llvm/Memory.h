//
// Created by slty5 on 24-11-25.
//

#ifndef MEMORY_H
#define MEMORY_H

namespace thm {
    class Value;

struct MemoryToken {
    Value *address;
};
class MemoryAccess {
public:
    MemoryToken memToken;

    MemoryAccess(Value* address);
};
class MemoryDef : MemoryAccess {

};
class MemoryUse : MemoryAccess {

};
class MemoryPhi : MemoryAccess {

};
} // thm

#endif //MEMORY_H
