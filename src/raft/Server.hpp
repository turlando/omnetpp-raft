#pragma once

#include "types.hpp"

namespace raft {

class Server {
    private:
        const GetServerId getServerId;
        const GetServers  getServers;
        const SendMessage sendMessage;

        enum ServerState state;
        int              term;

        void broadcast(Message);
        void election();

    public:
        Server(GetServerId getServerId, GetServers getServers, SendMessage sendMessage)
            : getServerId(getServerId)
            , getServers(getServers)
            , sendMessage(sendMessage)
            , state(Follower)
            , term(0)
        {};
};

}
