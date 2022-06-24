#include "types.hpp"

namespace raft {

template <typename LogAction>
struct LogEntry {
    Term      term;
    LogAction action;

    LogEntry(Term term, LogAction action)
        : term(term)
        , action(action)
    {};
};

template <typename LogAction>
class Log {
    private:
        std::vector<LogEntry<LogAction>> log;

    public:
        int lastIndex() {
            return log.size() - 1;
        }

        int lastTerm() {
            if (log.empty() == true)
                return -1;
            else
                return log.at(lastIndex()).term;
        }
};

}
