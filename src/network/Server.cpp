#include <functional>
#include <omnetpp.h>

#include "utils.hpp"
#include "msg/Heartbeat_m.h"
#include "msg/InternalPingTimeout_m.h"

#include "../raft/raft.hpp"


const float pingDeltaTime = 0.05; // 50 ms

class Server : public omnetpp::cSimpleModule {

private:
    int id;

    struct raft::Configuration raftConfiguration;
    struct raft::State         raftState;

    omnetpp::cGate *gateForNode(int);
    void broadcast(std::function<omnetpp::cMessage* (void)>);

    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *) override;

};

Define_Module(Server);

omnetpp::cMessage *raftMessageToOmnetMessage(raft::Message message) {
    return std::visit(match {
        [](raft::Heartbeat& x) { return new Heartbeat(); },
    }, message);
}

omnetpp::cGate *Server::gateForNode(int id) {
    if (id == this->id)
        throw omnetpp::cRuntimeError("Trying to send message to self");

    for (omnetpp::cModule::GateIterator i(this); !i.end(); ++i) {
        int nodeId = (*i)->getPathEndGate()->getOwnerModule()->getIndex();

        if ((*i)->getType() == omnetpp::cGate::OUTPUT && id == nodeId)
            return *i;
    }

    throw omnetpp::cRuntimeError("No outgoing channel");
}

void Server::broadcast(std::function<omnetpp::cMessage* (void)> mkMsg) {
    for (omnetpp::cModule::GateIterator i(this); !i.end(); ++i)
        if ((*i)->getType() == omnetpp::cGate::OUTPUT)
            send(mkMsg(), *i);

}

void Server::initialize() {
    id = getIndex();

    raftConfiguration = raft::Configuration(
        id,
        [&](int id, raft::Message msg) { send(raftMessageToOmnetMessage(msg), gateForNode(id)); }
    );

    scheduleAfter(pingDeltaTime, new InternalPingTimeout());

}

void Server::handleMessage(omnetpp::cMessage *msg) {
    InternalPingTimeout *ipt = dynamic_cast<InternalPingTimeout*>(msg);
    if (ipt != nullptr) {
        broadcast([](){ return new Heartbeat(); });
        scheduleAfter(pingDeltaTime, new InternalPingTimeout());
        return;
    }

}
