#include "Server.hpp"

#include "../utils.hpp"

namespace raft {

void Server::broadcast(Message message) {
    for (auto server : getServers())
        send(server, message);
}

void Server::election() {
    term         += 1;
    state         = Candidate;
    receivedVotes = 1; // voting for itself
    votedCandidate = getId();
    broadcast(RequestVote(term));
}

int Server::requiredVotesToBeLeader() {
    return (getServers().size() - 1) / 2;
}

void Server::handleMessage(ServerId from, Message message) {
    std::visit(match {
        [&](Heartbeat& x) {
            if (state == Candidate && from == votedCandidate) {
                state = Follower;
            }
        },

        [&](RequestVote& x) {
            if (x.term < term) {
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

        [&](RequestVoteReply& x) {
            if (x.agree == true)
                receivedVotes += 1;

            if (receivedVotes >= requiredVotesToBeLeader()) {
                state = Leader;
                broadcast(Heartbeat());
            }
        }
    }, message);
}

}
