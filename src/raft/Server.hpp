#pragma once

#include <optional>
#include "types.hpp"

namespace raft {

const Time HEARTBEAT_TIMEOUT = std::chrono::milliseconds(50);

class Server {
    private:
        const GetTime     getTime;
        const GetServerId getId;
        const GetServers  getServers;
        const SendMessage send;

        enum Role               role;
        Term                    term;
        Time                    lastHeartbeatTime;
        std::optional<ServerId> votedCandidate;
        int                     receivedVotes;

        void becomeFollower();
        void becomeCandidate();
        void becomeLeader();

        int  requiredVotesToBeLeader();
        bool isLeaderAlive();
        void broadcast(Message);
        void updateHeartbeatTime();
        void election();

    public:
        Server(GetTime getTime, GetServerId getId, GetServers getServers, SendMessage sendMessage)
            : getTime(getTime)
            , getId(getId)
            , getServers(getServers)
            , send(sendMessage)
            , role(Follower)
            , term(0)
            , lastHeartbeatTime() // initialized to epoch
            , votedCandidate(std::nullopt)
            , receivedVotes(0)
        {};

        enum Role getRole();
        Term      getTerm();
        int       getReceivedVotes();

        void handleMessage(ServerId from, Message message);
        void maybeElection();
        void maybeHeartbeat();
};

}
