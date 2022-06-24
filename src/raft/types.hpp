#pragma once

#include <functional>
#include <chrono>
#include <map>
#include <set>
#include <variant>

namespace raft {

/* Basic types ***************************************************************/

using Time    = std::chrono::milliseconds;
using GetTime = std::function<Time (void)>;

using ServerId = int;
using Term     = int;
using Servers  = std::set<ServerId>;

using GetServerId = std::function<ServerId (void)>;
using GetServers  = std::function<Servers (void)>;

using ResetElectionTimeout = std::function<void (void)>;

/* Message types *************************************************************/

enum Role {
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
    int  lastLogIndex; // TODO: use more meaningful types
    int  lastLogTerm;  // TODO: use more meaningful types

    RequestVote(Term term, int lastLogIndex, int lastLogTerm)
        : term(term)
        , lastLogIndex(lastLogIndex)
        , lastLogTerm(lastLogTerm)
    {};
};

struct RequestVoteReply {
    Term term;
    bool agree; // true = voted for sender

    RequestVoteReply(Term term, bool agree)
        : term(term)
        , agree(agree)
    {};
};

using Message     = std::variant<Heartbeat, RequestVote, RequestVoteReply>;
using SendMessage = std::function<void (ServerId, Message)>;

}
