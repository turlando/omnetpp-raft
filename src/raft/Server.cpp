#include "Server.hpp"

#include "../utils.hpp"

namespace raft {

/****************************************************************************/

enum Role               Server::getRole()          { return role; }
int                     Server::getReceivedVotes() { return receivedVotes; }
Term                    Server::getTerm()          { return term; }
std::optional<ServerId> Server::getLeader()        { return leader; }

/****************************************************************************/

int Server::quorum() {
    int nodes = getServers().size() + 1; // Adding itself
    return (nodes + 1) / 2;
}

bool Server::isLeaderAlive() {
    return (getTime() - lastHeartbeatTime) <= HEARTBEAT_TIMEOUT;
}

/****************************************************************************/

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

    nextIndex.clear();
    matchIndex.clear();

    for (auto server : getServers()) {
        nextIndex.insert_or_assign(server, log.size());
        matchIndex.insert_or_assign(server, 0);
    }

    role = Leader;

    broadcast(AppendEntries(term, log.lastIndex(), log.lastTerm(), {}, commitIndex));
}

/****************************************************************************/

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
        broadcast(AppendEntries(term, log.lastIndex(), log.lastTerm(), {}, commitIndex));
}

/****************************************************************************/

void Server::updateHeartbeatTime() {
    lastHeartbeatTime = getTime();
}

void Server::updateTerm(Term messageTerm) {
    if (messageTerm > term) {
        becomeFollower(messageTerm);
        updateHeartbeatTime();
    }
}

/****************************************************************************/

void Server::broadcast(Message message) {
    for (auto server : getServers())
        send(server, message);
}


void Server::election() {
    becomeCandidate();
    broadcast(RequestVote(term, log.lastIndex(), log.lastTerm()));
}

/****************************************************************************/

void Server::handleMessage(ServerId from, Message message) {
    std::visit(match {
        [&](RequestVote& msg) {
            updateTerm(msg.term);

            bool logOk
                = msg.lastLogTerm > log.lastTerm()
               || (   msg.lastLogTerm == log.lastTerm()
                   && msg.lastLogIndex >= log.lastIndex());

            bool grantVote
                = msg.term == term
               && logOk == true
               && (   votedCandidate.has_value() == false
                   || votedCandidate.value() == from);

            if (grantVote == true && role == Follower) {
                resetElectionTimeout();
                votedCandidate = from;
            }

            send(from, RequestVoteReply(term, grantVote));
        },

        [&](RequestVoteReply& msg) {
            if (msg.term != term)
                return; // TODO: use assert / log this condition?

            if (role != Candidate)
                return; // TODO: use assert / log this condition?

            if (msg.agree == true) {
                receivedVotes += 1;
            }

            if (receivedVotes >= quorum())
                becomeLeader();
        },

        [&](AppendEntries& msg) {
            updateHeartbeatTime();

            int prevLogTerm = log.getTerm(msg.prevLogIndex);

            bool logOk
                = msg.prevLogIndex == 0 // TODO: encapsulate default? Use optional?
               || msg.prevLogTerm == prevLogTerm;

            if (msg.term == term) {
                switch (role) {
                    case Candidate:
                        resetElectionTimeout();
                        becomeFollower(term, from);
                        break;

                    case Follower:
                        if (leader.has_value() == false) {
                            resetElectionTimeout();
                            becomeFollower(term, from);
                        }
                        break;

                    case Leader:
                        // TODO: signal illegal condition
                        break;
                }
            }

            if (msg.term < term) {
                send(from, AppendEntriesReply(term, false));
                return;
            }

            // TODO: use assert?
            if (   msg.term != term
                || role != Follower
                || logOk == false) {
                // TODO: log this condition?
                return;
            }

            // Discriminate heartbeat
            if (msg.entry.has_value() == false)
                return;

            if (msg.prevLogIndex == log.lastIndex()) {
                // Leader and follower are synchronized
                log.append(msg.entry.value());
            } else {
                int currentLogTerm = log.lastTerm();
                if (currentLogTerm != msg.entry.value().term) {
                    // TODO: assert(msg.prevLogIndex + 1 > commitIndex);
                    log.removeFrom(msg.prevLogIndex + 1);
                    log.append(msg.entry.value());
                }
            }

            commitIndex = std::min(commitIndex, msg.prevLogIndex + 1);
            send(from, AppendEntriesReply(term, true));
        },

        [&](AppendEntriesReply& msg) {
            if (msg.term != term)
                return; // TODO: use assert / log this condition

            if (role != Leader)
                return; // TODO: use assert / log this condition

            if (msg.success == true) {
                nextIndex.insert_or_assign(from, nextIndex.at(from) + 1);
                matchIndex.insert_or_assign(from, nextIndex.at(from) - 1);
            } else {
                nextIndex.insert_or_assign(from, nextIndex.at(from) - 1);
                send(from, AppendEntries(
                    term,
                    nextIndex.at(from) - 1,
                    log.get(nextIndex.at(from) - 1).term,
                    log.get(nextIndex.at(from)),
                    commitIndex
                ));
            }
        }
    }, message);
}

/****************************************************************************/

}
