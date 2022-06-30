#include "utils.hpp"

#include <variant>

#include <omnetpp/cexception.h>

#include "../raft/types.hpp"
#include "../utils.hpp"
#include "msg/AppendEntries_m.h"
#include "msg/AppendEntriesReply_m.h"
#include "msg/RequestVote_m.h"
#include "msg/RequestVoteReply_m.h"

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message) {
    return std::visit(match {
        [](raft::RequestVote& x) -> omnetpp::cMessage* {
            auto m = new RequestVote();
            m->setTerm(x.term);
            m->setLastLogIndex(x.lastLogIndex);
            m->setLastLogTerm(x.lastLogTerm);
            return m;
        },

        [](raft::RequestVoteReply& x) -> omnetpp::cMessage* {
            auto m = new RequestVoteReply();
            m->setTerm(x.term);
            m->setAgree(x.agree);
            return m;
        },

        [](raft::AppendEntries& x) -> omnetpp::cMessage* {
            auto m = new AppendEntries();
            m->setTerm(x.term);
            m->setPrevLogIndex(x.prevLogIndex);
            m->setPrevLogTerm(x.prevLogTerm);
            m->setEntry(x.entry);
            m->setLeaderCommit(x.leaderCommit);
            return m;
        },
        [](raft::AppendEntriesReply& x) -> omnetpp::cMessage* {
            auto m = new AppendEntriesReply();
            m->setTerm(x.term);
            m->setSuccess(x.success);
            return m;
        }
    }, message);

    throw omnetpp::cRuntimeError("Unable to convert Raft message to Omnet++ message");
}


raft::Message omnetMessageToRaftMessage(omnetpp::cMessage *message) {
    {
        RequestVote *m = dynamic_cast<RequestVote*>(message);
        if (m != nullptr)
            return raft::RequestVote(m->getTerm(), m->getLastLogIndex(), m->getLastLogTerm());
    }

    {
        RequestVoteReply *m = dynamic_cast<RequestVoteReply*>(message);
        if (m != nullptr)
            return raft::RequestVoteReply(m->getTerm(), m->getAgree());
    }

    {
        AppendEntries *m = dynamic_cast<AppendEntries*>(message);
        if (m != nullptr)
            return raft::AppendEntries(
                m->getTerm(), m->getPrevLogIndex(), m->getPrevLogTerm(),
                m->getEntry(), m->getLeaderCommit()
            );
    }

    {
        AppendEntriesReply *m = dynamic_cast<AppendEntriesReply*>(message);
        if (m != nullptr)
            return raft::AppendEntriesReply(m->getTerm(), m->getSuccess());
    }

    throw omnetpp::cRuntimeError("Unable to convert Omnet++ message to Raft message");
}
