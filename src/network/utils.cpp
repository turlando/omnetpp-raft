#include "utils.hpp"

#include <variant>
#include "msg/Heartbeat_m.h"

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message) {
    return std::visit(match {
        [](raft::Heartbeat& x) { return new Heartbeat(); },
    }, message);
}
