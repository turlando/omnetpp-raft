#pragma once

#include <optional>
#include "types.hpp"
#include "Log.hpp"

namespace raft {

const Time HEARTBEAT_TIMEOUT = std::chrono::milliseconds(50);

class Server {
    private:
        const GetTime              getTime;
        const GetServerId          getId;
        const GetServers           getServers;
        const SendMessage          send;
        const ResetElectionTimeout resetElectionTimeout;

        enum Role               role;
        Term                    term;
        std::optional<ServerId> leader;
        Time                    lastHeartbeatTime;
        std::optional<ServerId> votedCandidate;
        int                     receivedVotes;

        Log<DummyLogAction> log;
        int commitIndex;
        int lastApplied;

        std::map<ServerId, int> nextIndex;
        std::map<ServerId, int> matchIndex;

        void becomeFollower(Term term);
        void becomeFollower(Term term, ServerId leader);
        void becomeCandidate();
        void becomeLeader();

        int  requiredVotesToBeLeader();
        bool isLeaderAlive();
        void broadcast(Message);
        void updateHeartbeatTime();
        void election();

    public:
        Server(
                GetTime getTime,
                GetServerId getId,
                GetServers getServers,
                SendMessage sendMessage,
                ResetElectionTimeout resetElectionTimeout
              )
            : getTime(getTime)
            , getId(getId)
            , getServers(getServers)
            , send(sendMessage)
            , resetElectionTimeout(resetElectionTimeout)
            , role(Follower)
            , term(0)
            , leader(std::nullopt)
            , lastHeartbeatTime() // initialized to epoch
            , votedCandidate(std::nullopt)
            , receivedVotes(0)
        {};

        enum Role               getRole();
        Term                    getTerm();
        std::optional<ServerId> getLeader();
        int                     getReceivedVotes();

        void handleMessage(ServerId from, Message message);
        void maybeElection();
        void maybeHeartbeat();
};

}
