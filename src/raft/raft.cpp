#include "raft.hpp"
#include <iostream>

namespace raft {


Servers getServers(State &s) {
    Servers servers;

    for (auto& [k, _v] : s.serverHeartbeats)
        servers.insert(k);

    return servers;
}

void sendToAllServers(SendMessage sendMessage, Servers servers, Message message) {
    for (auto server : servers)
        sendMessage(server, message);
}

void election(struct Configuration &c, struct State &s) {
    s.state  = Candidate;
    s.term  += 1;
    //sendToAllServers(c.sendMessage, getServers(s), RequestVote());
}

}
