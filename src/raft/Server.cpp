#include "Server.hpp"

namespace raft {

void Server::sendToAllServers(Message message) {
    for (auto server : getServers())
        sendMessage(server, message);
}

}
