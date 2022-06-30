cplusplus {{
#include "../../raft/types.hpp"
typedef raft::MaybeDummyLogEntry Entry;
}}

// Thank you Omnet++ for all this mental gymnastics
// My mental sanity really needed all this
class Entry {
    @existingClass;
}

message AppendEntries {
    int   term;
    int   prevLogIndex;
    int   prevLogTerm;
    Entry entry;
    int   leaderCommit;
}