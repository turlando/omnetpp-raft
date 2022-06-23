#pragma once

#include <variant>

namespace raft {

struct Heartbeat {
    int term;

    Heartbeat(int term)
        : term(term)
    {}
};

struct RequestVote {
    int term;

    RequestVote(int term)
        : term(term)
    {};
};

struct RequestVoteReply {
    bool agree; // true = voted for sender

    RequestVoteReply(bool agree)
        : agree(agree)
    {};
};

using Message = std::variant<Heartbeat, RequestVote, RequestVoteReply>;

}
