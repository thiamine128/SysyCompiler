//
// Created by slty5 on 24-11-7.
//

#ifndef STACKTRACKER_H
#define STACKTRACKER_H
#include <unordered_map>

namespace thm {

class StackTracker {
public:
    std::unordered_map<int, int> offset;

    void reset();

};

} // thm

#endif //STACKTRACKER_H
