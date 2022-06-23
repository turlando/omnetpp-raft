#pragma once

#include <functional>
#include <chrono>
#include <map>
#include <set>
#include <variant>

namespace raft {

using Time             = std::chrono::time_point<std::chrono::steady_clock>;

using ServerId         = int;
using Term             = int;

using Servers          = std::set<ServerId>;
using ServerHeartbeats = std::map<ServerId, Time>;

using GetServerId      = std::function<ServerId (void)>;
using GetServers       = std::function<Servers (void)>;

enum ServerState {
    Follower,
    Candidate,
    Leader
};

struct Heartbeat {
    Term term;

    Heartbeat(Term term)
        : term(term)
    {}
};

struct RequestVote {
    Term term;

    RequestVote(Term term)
        : term(term)
    {};
};

struct RequestVoteReply {
    bool agree; // true = voted for sender

    RequestVoteReply(bool agree)
        : agree(agree)
    {};
};

using Message     = std::variant<Heartbeat, RequestVote, RequestVoteReply>;
using SendMessage = std::function<void (ServerId, Message)>;


}
