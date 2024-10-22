#include <string>

#include "util.h"

#include <unordered_map>
//
// Created by slty5 on 24-10-21.
//

std::unordered_map<char, char> slashMap = {
    {'a', 7},
    {'b', 8},
    {'t', 9},
    {'n', 10},
    {'v', 11},
    {'f', 12},
    {'\"', 34},
    {'\'', 39},
    {'\\', 92},
    {'0', 0}
};

std::string fromRaw(char const* raw) {
    std::string result;
    for (int i = 0; raw[i]; ++i) {
        if (raw[i] == '\\') {
            i++;
            result += slashMap[raw[i]];
        } else {
            result += raw[i];
        }
    }
    return result;
}