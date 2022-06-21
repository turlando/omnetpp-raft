#pragma once

#include <variant>

namespace raft {

struct Heartbeat {};

using Message = std::variant<Heartbeat>;

}
