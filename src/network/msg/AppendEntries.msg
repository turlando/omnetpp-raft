cplusplus {{
#include <vector>
#include "../../raft/types.hpp"
typedef raft::DummyLogEntries Entries;
}}

// Thank you Omnet++ for all this mental gymnastics
// My mental sanity really needed all this
class Entries {
    @existingClass;
}

message AppendEntries {
    int     term;
    int     prevLogIndex;
    int     prevLogTerm;
    Entries entries;
    int     leaderCommit;
}