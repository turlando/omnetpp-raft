#pragma once

#include <omnetpp.h>
#include "../raft/message.hpp"

template<class... Ts> struct match : Ts... { using Ts::operator()...; };
template<class... Ts> match(Ts...) -> match<Ts...>;

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message);
