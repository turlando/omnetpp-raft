#include "Server.hpp"

#include "../utils.hpp"

namespace raft {

void Server::handleMessage(ServerId from, Message message) {
    std::visit(match {
        [&](Heartbeat& msg) {
            updateHeartbeatTime();

            if (role == Candidate && msg.term >= term) {
                term = msg.term;
                becomeFollower();
            }
        },

        [&](RequestVote& msg) {
            if (role != Follower) {
                send(from, RequestVoteReply(term, false));
                return;
            }

            if (msg.term < term || votedCandidate.has_value() == true) {
                send(from, RequestVoteReply(term, false));
            } else {
                votedCandidate = from;
                send(from, RequestVoteReply(term, true));
            }
        },

        [&](RequestVoteReply& msg) {
            if (role != Candidate)
                return;

            if (msg.agree == true && msg.term == term) {
                receivedVotes += 1;
                if (receivedVotes >= requiredVotesToBeLeader()) {
                    becomeLeader();
                }
                return;
            }

            if (msg.agree == false && term < msg.term) {
                term = msg.term;
                becomeFollower();
                return;
            }
        }
    }, message);
}

enum Role Server::getRole()          { return role; }
int       Server::getReceivedVotes() { return receivedVotes; }
Term      Server::getTerm()          { return term; }

// this must only be called once every ELECTION_TIMEOUT
void Server::maybeElection() {
    // if last communication from leader has happened during election timeout
    // then start new election.
    // if election is in progress (state == Candidate) and no leader has been
    // chosen during election timeout then start a new election.
    if (role != Leader && isLeaderAlive() == false)
        election();
}

void Server::maybeHeartbeat() {
    if (role == Leader)
        broadcast(Heartbeat(term));
}

void Server::becomeFollower() {
    votedCandidate.reset();
    role = Follower;
}

void Server::becomeCandidate() {
    term           += 1;
    receivedVotes  =  1; // voting for itself
    votedCandidate =  getId();
    role           =  Candidate;
}

void Server::becomeLeader() {
    receivedVotes = 0;
    votedCandidate.reset();
    role = Leader;
}

int Server::requiredVotesToBeLeader() {
    return (getServers().size() + 1) / 2;
}

bool Server::isLeaderAlive() {
    return (getTime() - lastHeartbeatTime) <= HEARTBEAT_TIMEOUT;
}

void Server::broadcast(Message message) {
    for (auto server : getServers())
        send(server, message);
}

void Server::updateHeartbeatTime() {
    lastHeartbeatTime = getTime();
}

void Server::election() {
    becomeCandidate();
    broadcast(RequestVote(term));
}

}
