#pragma once

#include <functional>
#include <chrono>
#include <map>
#include <set>

#include "message.hpp"

namespace raft {

using Time             = std::chrono::time_point<std::chrono::steady_clock>;

using ServerId         = int;
using Term             = int;

using Servers          = std::set<ServerId>;
using ServerHeartbeats = std::map<ServerId, Time>;

using GetServerId      = std::function<ServerId (void)>;
using GetServers       = std::function<Servers (void)>;
using SendMessage      = std::function<void (ServerId, Message)>;

enum ServerState {
    Follower,
    Candidate,
    Leader
};

}
