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

/* Log types *****************************************************************/

template <typename LogAction>
struct LogEntry {
    Term      term;
    LogAction action;

    LogEntry() {}; // TODO: just to make omnet happy in AppenRequest.msg
    LogEntry(Term term, LogAction action)
        : term(term)
        , action(action)
    {};
};

template <typename LogAction>
using LogEntries = std::vector<LogEntry<LogAction>>;

struct DummyLogAction {};
using DummyLogEntries = LogEntries<DummyLogAction>;

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

struct AppendEntries {
    Term term;
    int prevLogIndex; // TODO: use more meaningful types
    int prevLogTerm;  // TODO: use more meaningful types
    DummyLogEntries entries;
    int leaderCommit; // TODO: use more meaningful types

    AppendEntries(
        Term term, int prevLogIndex, int prevLogTerm,
        DummyLogEntries entries, int leaderCommit
    )   : term(term)
        , prevLogIndex(prevLogIndex)
        , prevLogTerm(prevLogTerm)
        , entries(entries)
        , leaderCommit(leaderCommit)
    {};
};

using Message     = std::variant<Heartbeat, RequestVote, RequestVoteReply, AppendEntries>;
using SendMessage = std::function<void (ServerId, Message)>;

}
