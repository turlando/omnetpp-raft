#include "Server.hpp"

namespace raft {

void Server::broadcast(Message message) {
    for (auto server : getServers())
        sendMessage(server, message);
}

}
