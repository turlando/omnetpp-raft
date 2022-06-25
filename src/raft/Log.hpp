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

        Term getTerm(int index) {
            if (log.empty() == true)
                return -1;
            else
                return log.at(index).term;
        }

        int lastIndex() {
            return log.size() - 1;
        }

        int lastTerm() {
            return getTerm(lastIndex());
        }

        void removeFrom(int index) {
            log.erase(log.begin() + index, log.end());
        }

        void insertFrom(int index, std::vector<LogEntry<LogAction>> entries) {
            log.insert(log.begin() + index, entries.begin(), entries.end());
        }

        void insertAt(int index, LogEntry<LogAction> entry) {
            log.insert(log.begin() + index, entry);
        }
};

}
