#include <set>
#include <utility>

#include <omnetpp.h>

#include "utils.hpp"
#include "msg/InternalElectionTimeout_m.h"
#include "msg/InternalHeartbeatTimeout_m.h"
#include "../raft/Server.hpp"


// TODO: unify with timeouts from raft module
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

        InternalElectionTimeout *internalElectionTimeout;

        raft::Time      getTime();
        raft::Servers   getServers();
        omnetpp::cGate* gateForNode(int);
        void            sendRaftMessageToNode(raft::ServerId id, raft::Message msg);

        void resetElectionTimeout();

        void setColor(const char *color);
        void setText(const char *color);
        void updateDisplay();

        omnetpp::simsignal_t messageReceivedSignal = registerSignal("messageReceived");
        omnetpp::simsignal_t messageSentSignal = registerSignal("messageSent");

    protected:
        virtual void initialize() override;
        virtual void handleMessage(omnetpp::cMessage *) override;

    public:
        Server()
            : omnetpp::cSimpleModule()
            , internalElectionTimeout(new InternalElectionTimeout())
            , raftServer(
                [&]() { return getTime(); },
                [&]() { return getId(); },
                [&]() { return getServers(); },
                [&](int id, raft::Message msg) { sendRaftMessageToNode(id, msg); },
                [&]() { resetElectionTimeout(); }
            )
        {};
};

Define_Module(Server);

void Server::initialize() {
    updateDisplay();

    resetElectionTimeout();
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

    // If it's not an internal event then fire the messageReceived signal
    emit(messageReceivedSignal, 0);

    raft::Message m = omnetMessageToRaftMessage(msg);
    raftServer.handleMessage(msg->getSenderGate()->getOwnerModule()->getId(), m);

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

    switch (raftServer.getRole()) {
        case raft::Follower:
            snprintf(buffer, sizeof(buffer),
                     "Term: %d\nLeader: %d",
                     raftServer.getTerm(),
                     raftServer.getLeader().value_or(-1));
            break;
        case raft::Candidate:
            snprintf(buffer, sizeof(buffer),
                     "Term: %d\nReceivedVotes: %d",
                     raftServer.getTerm(),
                     raftServer.getReceivedVotes());
            break;
        default:
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
        cModule *module = (*i)->getPathEndGate()->getOwnerModule();
        const char *name = module->getClassName();
        const int id = module->getId();

        if (strcmp(name, "Server") == 0 && id != getId())
            s.insert(id);
    }

    return s;
}

omnetpp::cGate *Server::gateForNode(raft::ServerId _id) {
    if (_id == getId())
        throw omnetpp::cRuntimeError("Trying to send message to self");

    for (omnetpp::cModule::GateIterator i(this); !i.end(); ++i) {
        omnetpp::cGate::Type type = (*i)->getType();
        cModule *module = (*i)->getPathEndGate()->getOwnerModule();
        const int id = module->getId();

        if (type == omnetpp::cGate::OUTPUT && id == _id)
            return *i;
    }

    throw omnetpp::cRuntimeError("No outgoing channel");
}

void Server::sendRaftMessageToNode(raft::ServerId id, raft::Message msg) {
    emit(messageSentSignal, 0);
    omnetpp::cMessage *omnetMessage = raftMessageToOmnetMessage(msg);
    omnetpp::cGate    *destGate     = gateForNode(id);
    send(omnetMessage, destGate);
    updateDisplay();

}

void Server::resetElectionTimeout() {
    cancelEvent(internalElectionTimeout);
    scheduleAfter(uniform(startupElectionMinTimeout, startupElectionMaxTimeout),
                  internalElectionTimeout);
}
