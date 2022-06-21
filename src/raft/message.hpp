#pragma once

#include <variant>

namespace raft {

struct Heartbeat {};

struct RequestVote {
    int term;

    RequestVote(int term)
        : term(term)
    {};
};

struct RequestVoteReply {};

using Message = std::variant<Heartbeat, RequestVote>;

}
