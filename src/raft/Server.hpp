#pragma once

#include <chrono>
#include <map>
#include <optional>

#include "Log.hpp"
#include "types.hpp"

namespace raft {

const Time HEARTBEAT_TIMEOUT = std::chrono::milliseconds(50);

class Server {
    private:
        /********************************************************************/

        const GetTime              getTime;
        const GetServerId          getId;
        const GetServers           getServers;
        const SendMessage          send;
        const ResetElectionTimeout resetElectionTimeout;

        /********************************************************************/

        enum Role               role;
        std::optional<ServerId> leader;
        Time                    lastHeartbeatTime;
        int                     receivedVotes;

        /*
         * Persistent state on all servers
         * Updated on stable storage before responding to RPCs
         */
        Term                    term;
        std::optional<ServerId> votedCandidate;
        Log<DummyLogAction>     log;


        /*
         * Volatile state on all servers.
         * Actually, not really,
         * cfr. https://groups.google.com/g/raft-dev/c/KIozjYuq5m0
         */
        int commitIndex;
        int lastApplied;

        /*
         * Volatile state on leaders.
         * Reinitialized after election.
         */
        std::map<ServerId, int> nextIndex;
        std::map<ServerId, int> matchIndex;

        /********************************************************************/

        int  quorum();
        bool isLeaderAlive();

        void becomeFollower(Term term);
        void becomeFollower(Term term, ServerId leader);
        void becomeCandidate();
        void becomeLeader();

        void updateHeartbeatTime();
        void updateTerm(Term messageTerm);

        void broadcast(Message msg);
        void election();

        /********************************************************************/

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
        {};

        enum Role               getRole();
        Term                    getTerm();
        std::optional<ServerId> getLeader();
        int                     getReceivedVotes();

        void maybeElection();
        void maybeHeartbeat();
        void handleMessage(ServerId from, Message message);
};

}
