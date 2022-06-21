#pragma once

#include <functional>
#include <chrono>
#include <map>
#include <set>

#include "message.hpp"

namespace raft {

using ServerId         = int;
using Term             = int;
using SendMessage      = std::function<void (ServerId, Message)>;
using MonotonicClock   = std::chrono::steady_clock;
using ServerHeartbeats = std::map<ServerId, MonotonicClock>;
using Servers          = std::set<ServerId>;

}
