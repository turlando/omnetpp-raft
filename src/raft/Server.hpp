#pragma once

#include <optional>
#include "types.hpp"

namespace raft {

class Server {
    private:
        const GetServerId getId;
        const GetServers  getServers;
        const SendMessage send;

        enum ServerState        state;
        Term                    term;
        std::optional<ServerId> votedCandidate;
        int                     receivedVotes;

        void broadcast(Message);
        int requiredVotesToBeLeader();


    public:
        Server(GetServerId getId, GetServers getServers, SendMessage sendMessage)
            : getId(getId)
            , getServers(getServers)
            , send(sendMessage)
            , state(Follower)
            , term(0)
            , votedCandidate(std::nullopt)
            , receivedVotes(0)
        {};

        void election();
        void handleMessage(ServerId from, Message message);
};

}
