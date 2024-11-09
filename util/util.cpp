#include <string>

#include "util.h"

#include <iostream>
#include <unordered_map>
//
// Created by slty5 on 24-10-21.
//

std::unordered_map<char, char> escapeMap = {
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

std::unordered_map<char, std::string> unescapeMap = {
    {'\a', "\\a"},
    {'\b', "\\b"},
    {'\t', "\\t"},
    {'\n', "\\n"},
    {'\v', "\\v"},
    {'\f', "\\f"},
    {'\"', "\\\""},
    {'\'', "\\\'"},
    {'\\', "\\\\"},
    {'\0', "\\0"}
};

std::string fromRaw(char const* raw) {
    std::string result;
    for (int i = 1; raw[i + 1]; ++i) {
        if (raw[i] == '\\') {
            i++;
            result += escapeMap[raw[i]];
        } else {
            result += raw[i];
        }
    }
    return result;
}

int escape(char ch) {
    return escapeMap[ch];
}

std::string unescape(char ch) {
    if (unescapeMap.find(ch) == unescapeMap.end()) {
        return "" + ch;
    }
    return unescapeMap[ch];
}