#include "utils.hpp"

std::chrono::time_point<std::chrono::steady_clock> now() {
    return std::chrono::steady_clock::now();
}
