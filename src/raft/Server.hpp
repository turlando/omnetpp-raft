#pragma once

#include <optional>
#include "types.hpp"

namespace raft {

const std::chrono::milliseconds HEARTBEAT_TIMEOUT = std::chrono::milliseconds(50);

class Server {
    private:
        const GetServerId getId;
        const GetServers  getServers;
        const SendMessage send;

        enum ServerState        state;
        Term                    term;
        Time                    lastHeartbeatTime;
        std::optional<ServerId> votedCandidate;
        int                     receivedVotes;

        int requiredVotesToBeLeader();
        void broadcast(Message);
        void election();

    public:
        Server(GetServerId getId, GetServers getServers, SendMessage sendMessage)
            : getId(getId)
            , getServers(getServers)
            , send(sendMessage)
            , state(Follower)
            , term(0)
            , lastHeartbeatTime() // initialized to epoch
            , votedCandidate(std::nullopt)
            , receivedVotes(0)
        {};

        void handleMessage(ServerId from, Message message);
        void maybeElection();
        void maybeHeartbeat();
};

}
