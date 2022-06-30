#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <set>
#include <variant>
#include <vector>

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
using DummyLogEntry = LogEntry<DummyLogAction>;
using MaybeDummyLogEntry = std::optional<DummyLogEntry>;

/* Message types *************************************************************/

enum Role {
    Follower,
    Candidate,
    Leader
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
    MaybeDummyLogEntry entry;
    int leaderCommit; // TODO: use more meaningful types

    AppendEntries(
        Term term, int prevLogIndex, int prevLogTerm,
        MaybeDummyLogEntry entry, int leaderCommit
    )   : term(term)
        , prevLogIndex(prevLogIndex)
        , prevLogTerm(prevLogTerm)
        , entry(entry)
        , leaderCommit(leaderCommit)
    {};
};

struct AppendEntriesReply {
    Term term;
    bool success;

    AppendEntriesReply(Term term, bool success)
        : term(term)
        , success(success)
    {};
};

using Message     = std::variant<RequestVote, RequestVoteReply, AppendEntries, AppendEntriesReply>;
using SendMessage = std::function<void (ServerId, Message)>;

}
