#pragma once

#include "types.hpp"

namespace raft {

class Server {
    private:
        ServerId    id;
        GetServers  getServers;
        SendMessage sendMessage;

        enum ServerState state;
        int              term;

        void sendToAllServers(Message);

    public:
        Server(ServerId id, GetServers getServers, SendMessage sendMessage)
            : id(id)
            , getServers(getServers)
            , sendMessage(sendMessage)
            , state(Follower)
            , term(0)
        {};
};

}
