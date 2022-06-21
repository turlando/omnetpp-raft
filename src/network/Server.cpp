#include <functional>
#include <omnetpp.h>

#include "utils.hpp"
#include "msg/Heartbeat_m.h"
#include "msg/InternalElectionTimeout_m.h"

#include "../raft/Server.hpp"

const float startupElectionMinTimeout = 0.15; // 150 ms
const float startupElectionMaxTimeout = 0.30; // 300 ms

const float pingTimeout     = 0.05; // 50 ms
const float electionTimeout = 0.30; // 300 ms

class Server : public omnetpp::cSimpleModule {
    private:
        raft::Server raftServer;

        virtual void initialize() override;
        virtual void handleMessage(omnetpp::cMessage *) override;

        raft::Servers getServers();
        omnetpp::cGate *gateForNode(int);
        void broadcast(std::function<omnetpp::cMessage* (void)>);

    public:
        Server()
            : omnetpp::cSimpleModule()
            , raftServer(
                [&]() { return getIndex(); },
                [&]() { return getServers(); },
                [&](int id, raft::Message msg) { send(raftMessageToOmnetMessage(msg), gateForNode(id)); }
            )
        {};
};

Define_Module(Server);

omnetpp::cGate *Server::gateForNode(int id) {
    if (id == getIndex())
        throw omnetpp::cRuntimeError("Trying to send message to self");

    for (omnetpp::cModule::GateIterator i(this); !i.end(); ++i) {
        int nodeId = (*i)->getPathEndGate()->getOwnerModule()->getIndex();

        if ((*i)->getType() == omnetpp::cGate::OUTPUT && id == nodeId)
            return *i;
    }

    throw omnetpp::cRuntimeError("No outgoing channel");
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

void Server::broadcast(std::function<omnetpp::cMessage* (void)> mkMsg) {
    for (omnetpp::cModule::GateIterator i(this); !i.end(); ++i)
        if ((*i)->getType() == omnetpp::cGate::OUTPUT)
            send(mkMsg(), *i);

}

void Server::initialize() {
    scheduleAfter(uniform(startupElectionMinTimeout, startupElectionMaxTimeout),
                  new InternalElectionTimeout());
}

void Server::handleMessage(omnetpp::cMessage *msg) {
    InternalElectionTimeout *iet = dynamic_cast<InternalElectionTimeout*>(msg);
    if (iet != nullptr) {
        raftServer.election();
        return;
    }
}
