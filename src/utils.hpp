#pragma once

#include <chrono>

template<class... Ts> struct match : Ts... { using Ts::operator()...; };
template<class... Ts> match(Ts...) -> match<Ts...>;

std::chrono::time_point<std::chrono::steady_clock> now();
