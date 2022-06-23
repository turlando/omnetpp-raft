#include "Server.hpp"

#include "../utils.hpp"

namespace raft {

void Server::broadcast(Message message) {
    for (auto server : getServers())
        send(server, message);
}

void Server::election() {
    term          += 1;
    state          = Candidate;
    receivedVotes  = 1; // voting for itself
    votedCandidate = getId();
    broadcast(RequestVote(term));
}

int Server::requiredVotesToBeLeader() {
    return (getServers().size() + 1) / 2;
}

// this must only be called once every ELECTION_TIMEOUT
void Server::maybeElection() {
    // if last communication from leader has happened during election timeout
    // then start new election.
    // if election is in progress (state == Candidate) and no leader has been
    // chosen during election timeout then start a new election.
    if (state != Leader && now() - lastHeartbeatTime > HEARTBEAT_TIMEOUT)
        election();
}

void Server::maybeHeartbeat() {
    if (state == Leader)
        broadcast(Heartbeat(term));
}

void Server::handleMessage(ServerId from, Message message) {
    std::visit(match {
        [&](Heartbeat& msg) {
            lastHeartbeatTime = now();

            if (state == Candidate && msg.term >= term) {
                state = Follower;
            }
        },

        [&](RequestVote& msg) {
            if (msg.term < term) {
                send(from, RequestVoteReply(false));
                return;
            }

            else if (votedCandidate.has_value() == false) {
                send(from, RequestVoteReply(true));
                votedCandidate = from;
                return;
            }

            else if (votedCandidate.has_value() == true && votedCandidate.value() == from) {
                send(from, RequestVoteReply(true));
                return;
            }

            else {
                send(from, RequestVoteReply(false));
                return;
            }
        },

        [&](RequestVoteReply& msg) {
            if (msg.agree == true)
                receivedVotes += 1;

            if (state != Leader && receivedVotes >= requiredVotesToBeLeader()) {
                state = Leader;
                // start broadcasting heartbeats
            }
        }
    }, message);
}

}
