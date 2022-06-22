#pragma once

#include <omnetpp.h>
#include "../raft/message.hpp"

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message);
raft::Message omnetMessageToRaftMessage(omnetpp::cMessage *message);
