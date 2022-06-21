#include "utils.hpp"

#include <variant>
#include "../raft/message.hpp"
#include "msg/Heartbeat_m.h"
#include "msg/RequestVote_m.h"

//network/utils.cpp
//network/utils.cpp: In lambda function:
//network/utils.cpp:10:76: error: could not convert '(operator new(160), (<statement>, ((Heartbeat*)<anonymous>)))' from 'Heartbeat*' to 'omnetpp::cMessage'
//   10 |         [](raft::Heartbeat& x) -> omnetpp::cMessage { return new Heartbeat(); },
//      |                                                                            ^
//      |                                                                            |
//      |                                                                            Heartbeat*
//network/utils.cpp: In lambda function:
//network/utils.cpp:15:20: error: could not convert 'm' from 'RequestVote*' to 'omnetpp::cMessage'
//   15 |             return m;
//      |                    ^
//      |                    |
//      |                    RequestVote*
//network/utils.cpp: In function 'omnetpp::cMessage* raftMessageToOmnetMessage(raft::Message)':
//network/utils.cpp:9:22: error: cannot convert 'std::__detail::__variant::__visit_result_t<match<raftMessageToOmnetMessage(raft::Message)::<lambda(raft::Heartbeat&)>, raftMessageToOmnetMessage(raft::Message)::<lambda(raft::RequestVote&)> >, std::variant<raft::Heartbeat, raft::RequestVote>&>' {aka 'omnetpp::cMessage'} to 'omnetpp::cMessage*' in return
//    9 |     return std::visit(match {
//      |            ~~~~~~~~~~^~~~~~~~
//      |                      |
//      |                      std::__detail::__variant::__visit_result_t<match<raftMessageToOmnetMessage(raft::Message)::<lambda(raft::Heartbeat&)>, raftMessageToOmnetMessage(raft::Message)::<lambda(raft::RequestVote&)> >, std::variant<raft::Heartbeat, raft::RequestVote>&> {aka omnetpp::cMessage}
//   10 |         [](raft::Heartbeat& x) -> omnetpp::cMessage { return new Heartbeat(); },
//      |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   11 |
//      |
//   12 |         [](raft::RequestVote& x) -> omnetpp::cMessage {
//      |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   13 |             auto m = new RequestVote();
//      |             ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   14 |             m->setTerm(x.term);
//      |             ~~~~~~~~~~~~~~~~~~~
//   15 |             return m;
//      |             ~~~~~~~~~
//   16 |         }
//      |         ~
//   17 |     }, message);
//      |     ~~~~~~~~~~~

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message) {
    return std::visit(match {
        [](raft::Heartbeat& x) -> omnetpp::cMessage { return new Heartbeat(); },

        [](raft::RequestVote& x) -> omnetpp::cMessage {
            auto m = new RequestVote();
            m->setTerm(x.term);
            return m;
        }
    }, message);
}
