#pragma once

#include <omnetpp.h>
#include "../raft/message.hpp"

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message);

template<class... Ts> struct match : Ts... { using Ts::operator()...; };
template<class... Ts> match(Ts...) -> match<Ts...>;
