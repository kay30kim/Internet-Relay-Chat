#pragma once

#include "Server.hpp"
#include <deque>
#include <map>

struct TickRecord {
    int client_fd;
    std::string message;
    double timestamp;
};

class LagCompensation {
public:
    static LagCompensation& getInstance();

    void logClientMessage(int client_fd, const std::string& message);
    std::deque<TickRecord> getValidMessages(int client_fd);
    void clearOldRecords();

private:
    std::map<int, std::deque<TickRecord> > message_logs;
    double getCurrentTime();
};
