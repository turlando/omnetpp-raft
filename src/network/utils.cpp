#include "utils.hpp"

#include <variant>
#include "../raft/message.hpp"
#include "msg/Heartbeat_m.h"
#include "msg/RequestVote_m.h"

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message) {
    return std::visit(match {
        [](raft::Heartbeat& x) -> omnetpp::cMessage* { return new Heartbeat(); },

        [](raft::RequestVote& x) -> omnetpp::cMessage* {
            auto m = new RequestVote();
            m->setTerm(x.term);
            return m;
        }
    }, message);
}
