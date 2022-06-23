#include <functional>
#include <omnetpp.h>

#include "utils.hpp"
#include "msg/Heartbeat_m.h"
#include "msg/InternalElectionTimeout_m.h"
#include "msg/InternalHeartbeatTimeout_m.h"

#include "../raft/Server.hpp"

const float startupElectionMinTimeout = 0.15; // 150 ms
const float startupElectionMaxTimeout = 0.30; // 300 ms
const float heartbeatTimeout          = 0.05; // 50 ms

const char *STATE_TO_COLOR[3] = {
    "#00FF00", // follower  = green
    "#FFFF00", // candidate = yellow
    "#0000FF"  // leader    = blue
};

class Server : public omnetpp::cSimpleModule {
    private:
        raft::Server raftServer;

        raft::Time      getTime();
        raft::Servers   getServers();
        omnetpp::cGate* gateForNode(int);
        void            sendRaftMessageToNode(raft::ServerId id, raft::Message msg);

        void setColor(const char *color);
        void setText(const char *color);
        void updateDisplay();

        virtual void initialize() override;
        virtual void handleMessage(omnetpp::cMessage *) override;

    public:
        Server()
            : omnetpp::cSimpleModule()
            , raftServer(
                [&]() { return getTime(); },
                [&]() { return getIndex(); },
                [&]() { return getServers(); },
                [&](int id, raft::Message msg) { sendRaftMessageToNode(id, msg); }
            )
        {};
};

Define_Module(Server);

void Server::initialize() {
    updateDisplay();

    scheduleAfter(uniform(startupElectionMinTimeout, startupElectionMaxTimeout),
                  new InternalElectionTimeout());
    scheduleAfter(heartbeatTimeout, new InternalHeartbeatTimeout());
}

void Server::handleMessage(omnetpp::cMessage *msg) {
    InternalElectionTimeout *iet = dynamic_cast<InternalElectionTimeout*>(msg);
    if (iet != nullptr) {
        raftServer.maybeElection();
        scheduleAfter(uniform(startupElectionMinTimeout, startupElectionMaxTimeout),
                      new InternalElectionTimeout());
        return;
    }

    InternalHeartbeatTimeout *iht = dynamic_cast<InternalHeartbeatTimeout*>(msg);
    if (iht != nullptr) {
        raftServer.maybeHeartbeat();
        scheduleAfter(heartbeatTimeout, new InternalHeartbeatTimeout());
        return;
    }

    raft::Message m = omnetMessageToRaftMessage(msg);
    raftServer.handleMessage(msg->getSenderGate()->getOwnerModule()->getIndex(), m);

    updateDisplay();
}

void Server::setColor(const char *color) {
    getDisplayString().setTagArg("b", 3, color);
}

void Server::setText(const char *text) {
    getDisplayString().setTagArg("t", 0, text);
}

void Server::updateDisplay() {
    setColor(STATE_TO_COLOR[raftServer.getRole()]);

    char buffer[512];

    if (raftServer.getRole() == raft::Candidate) {
        snprintf(buffer, sizeof(buffer),
                "Term: %d\nReceivedVotes: %d",
                raftServer.getTerm(), raftServer.getReceivedVotes());
    } else {
        snprintf(buffer, sizeof(buffer),
                "Term: %d\n",
                raftServer.getTerm());
    }

    setText(buffer);
}

raft::Time Server::getTime() {
    int64_t milliseconds = omnetpp::simTime().inUnit(omnetpp::SimTimeUnit::SIMTIME_MS);
    return std::chrono::milliseconds(milliseconds);
}

raft::Servers Server::getServers() {
    raft::Servers s;
    for (omnetpp::cModule::GateIterator i(this); !i.end(); i++) {
        int nodeId = (*i)->getPathEndGate()->getOwnerModule()->getIndex();
        if (nodeId != getIndex())
            s.insert(nodeId);
    }
    return s;
}

omnetpp::cGate *Server::gateForNode(raft::ServerId id) {
    if (id == getIndex())
        throw omnetpp::cRuntimeError("Trying to send message to self");

    for (omnetpp::cModule::GateIterator i(this); !i.end(); ++i) {
        int nodeId = (*i)->getPathEndGate()->getOwnerModule()->getIndex();

        if ((*i)->getType() == omnetpp::cGate::OUTPUT && id == nodeId)
            return *i;
    }

    throw omnetpp::cRuntimeError("No outgoing channel");
}

void Server::sendRaftMessageToNode(raft::ServerId id, raft::Message msg) {
    omnetpp::cMessage *omnetMessage = raftMessageToOmnetMessage(msg);
    omnetpp::cGate    *destGate     = gateForNode(id);
    send(omnetMessage, destGate);
    updateDisplay();

}

