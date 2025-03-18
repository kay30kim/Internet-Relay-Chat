#pragma once

#include "Server.hpp"
#include <deque>
#include <map>
#include <pthread.h>

struct TickRecord {
    int client_fd;
    std::string message;
    double timestamp;
};

class LagCompensation {
private:
    LagCompensation() { pthread_mutex_init(&log_mutex, nullptr); }  
    ~LagCompensation() { pthread_mutex_destroy(&log_mutex); }
    LagCompensation(const LagCompensation&) = delete;
    LagCompensation& operator=(const LagCompensation&) = delete;
    std::map<int, std::deque<TickRecord> > message_logs;
    double getCurrentTime();
    void clearOldRecords();
    pthread_mutex_t log_mutex;

public:
    static LagCompensation& getInstance();
    void logClientMessage(int client_fd, const std::string& message);
    std::deque<TickRecord> getValidMessages(int client_fd);
    void processClientMessage(Server* server, int client_fd, const std::string& message);
};
