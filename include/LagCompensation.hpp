#pragma once

#include "Server.hpp"
#include <deque>
#include <map>
#include <atomic>

#ifdef __linux__
#include <linux/futex.h>
#else
#include <pthread.h>
#endif

struct TickRecord {
    int client_fd;
    std::string message;
    double timestamp;
};

class LagCompensation {
private:
    LagCompensation() = default;
    ~LagCompensation() = default;
    LagCompensation(const LagCompensation&) = delete;
    LagCompensation& operator=(const LagCompensation&) = delete;

    std::map<int, std::deque<TickRecord> > message_logs;
    double getCurrentTime();

#ifdef __linux__
    // Linux-specific futex functions
    void futex_wait(std::atomic<int>& futex_var, int expected);
    void futex_wake(std::atomic<int>& futex_var);
#else
    // macOS alternative using pthread
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    void futex_wait(std::atomic<int>& futex_var, int expected);
    void futex_wake(std::atomic<int>& futex_var);
#endif

public:
    static LagCompensation& getInstance();
    void logClientMessage(int client_fd, const std::string& message);
    std::deque<TickRecord> getValidMessages(int client_fd);
    void clearOldRecords();
    void processClientMessage(Server* server, int client_fd, const std::string& message);
};
