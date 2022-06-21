#pragma once

#include "types.hpp"

namespace raft {

enum MachineState {
    Follower,
    Candidate,
    Leader
};

// Immutable state
struct Configuration {
    ServerId    id;
    SendMessage sendMessage;

    Configuration() {};
    Configuration(ServerId id, SendMessage sendMessage)
        : id(id)
        , sendMessage(sendMessage)
    {};
};

struct State {
     enum MachineState state;
     int               term;
     ServerHeartbeats  serverHeartbeats;

     State()
         : state(Follower)
         , term(0)
         , serverHeartbeats(ServerHeartbeats())
     {};
};

Servers getServers(struct State &s);
void sendToAllServers(SendMessage sendMessage, Servers servers, Message message);
void election(struct Configuration &c, struct State &s);

}
