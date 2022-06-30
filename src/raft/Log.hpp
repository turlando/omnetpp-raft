#include <vector>
#include "types.hpp"

namespace raft {

template <typename LogAction>
class Log {
    private:
        std::vector<LogEntry<LogAction>> log;

    public:
        int size() {
            return log.size();
        }

        LogEntry<LogAction> get(int index) {
            return log.at(index);
        }

        // TODO: return optional instead of -1.
        Term getTerm(int index) {
            if (log.empty() == true)
                return -1;
            else
                return log.at(index).term;
        }

        // TODO: This will return -1 if log is empty.
        // Return optional instead.
        int lastIndex() {
            return size() - 1;
        }

        int lastTerm() {
            return getTerm(lastIndex());
        }

        void removeFrom(int index) {
            log.erase(log.begin() + index, log.end());
        }

        void append(LogEntry<LogAction> entry) {
            log.push_back(entry);
        }
};

}
