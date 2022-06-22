#include "utils.hpp"

#include "../utils.hpp"
#include "../raft/message.hpp"
#include "msg/Heartbeat_m.h"
#include "msg/RequestVote_m.h"
#include "msg/RequestVoteReply_m.h"

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message) {
    return std::visit(match {
        [](raft::Heartbeat& x) -> omnetpp::cMessage* { return new Heartbeat(); },

        [](raft::RequestVote& x) -> omnetpp::cMessage* {
            auto m = new RequestVote();
            m->setTerm(x.term);
            return m;
        },

        [](raft::RequestVoteReply& x) -> omnetpp::cMessage* {
            auto m = new RequestVoteReply();
            m->setAgree(x.agree);
            return m;
        }
    }, message);
}


raft::Message omnetMessageToRaftMessage(omnetpp::cMessage *message) {
    {
        Heartbeat *m = dynamic_cast<Heartbeat*>(message);
        if (m != nullptr)
            return raft::Heartbeat();
    }

    {
        RequestVote *m = dynamic_cast<RequestVote*>(message);
        if (m != nullptr)
            return raft::RequestVote(m->getTerm());
    }

    {
        RequestVoteReply *m = dynamic_cast<RequestVoteReply*>(message);
        if (m != nullptr)
            return raft::RequestVoteReply(m->getAgree());
    }

    throw omnetpp::cRuntimeError("Unable to convert Omnet++ message to Raft message");
}
