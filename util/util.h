//
// Created by slty5 on 24-10-11.
//

#ifndef UTIL_H
#define UTIL_H

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


std::string fromRaw(char const* raw);

#endif //UTIL_H
