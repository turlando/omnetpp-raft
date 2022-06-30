#pragma once

template<class... Ts> struct match : Ts... { using Ts::operator()...; };
template<class... Ts> match(Ts...) -> match<Ts...>;
