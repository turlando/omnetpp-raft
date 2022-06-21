#pragma once

#include <variant>

namespace raft {

struct Heartbeat {};
struct RequestVote { int term; };
struct RequestVoteReply {};

using Message = std::variant<Heartbeat, RequestVote>;

}
