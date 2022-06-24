#include "Server.hpp"

#include <iostream>
#include "../utils.hpp"

namespace raft {

void Server::handleMessage(ServerId from, Message message) {
    std::visit(match {
        [&](Heartbeat& msg) {
            if (msg.term >= term) {
                becomeFollower(msg.term, from);
            }

            if (role == Follower && leader.has_value())
                updateHeartbeatTime();
        },

        [&](RequestVote& msg) {
            bool logOk
                = (msg.lastLogTerm > log.lastTerm())
               || (msg.lastLogTerm == log.lastTerm()
                   && msg.lastLogIndex >= log.lastIndex());

            bool grantVote
                = msg.term == term
               && logOk == true
               && votedCandidate.has_value() == false
               || (votedCandidate.has_value() == true && votedCandidate.value() == from);

            if (role == Follower)
                resetElectionTimeout();

            send(from, RequestVoteReply(term, grantVote));
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
                becomeFollower(msg.term);
                return;
            }
        },
        [&](AppendEntries& msg) {
            if (msg.term < term) {
                // reply false
                return;
            }

            bool logOk
                = msg.prevLogIndex == -1
               || msg.prevLogTerm == log.getTerm(msg.prevLogIndex);

            if (logOk == false) {
                // reply false
                return;
            }

            // might be broken
            log.removeFrom(msg.prevLogIndex);
            log.insertFrom(msg.prevLogIndex, msg.entries);

            if (msg.leaderCommit > commitIndex)
                commitIndex = std::min(msg.leaderCommit, log.lastIndex());

        }
    }, message);
}

enum Role               Server::getRole()          { return role; }
int                     Server::getReceivedVotes() { return receivedVotes; }
Term                    Server::getTerm()          { return term; }
std::optional<ServerId> Server::getLeader()        { return leader; }

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

void Server::becomeFollower(Term _term) {
    votedCandidate.reset();
    leader.reset();
    term = _term;
    role = Follower;
}

void Server::becomeFollower(Term _term, ServerId _leader) {
    votedCandidate.reset();
    term = _term;
    leader = _leader;
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
    int nodes = getServers().size() + 1; // Adding itself
    return (nodes + 1) / 2;
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
    broadcast(RequestVote(term, log.lastIndex(), log.lastTerm()));
}

}
