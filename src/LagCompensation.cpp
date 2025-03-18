#include "LagCompensation.hpp"
#include "Server.hpp"
#include <chrono>
#include <sys/time.h>
#include <atomic>
#include <unistd.h>
#include <pthread.h>

// Mutex and condition variable for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Singleton instance retrieval
LagCompensation& LagCompensation::getInstance() {
    static LagCompensation instance;
    return instance;
}

// Returns the current time in seconds
double LagCompensation::getCurrentTime() {
    using namespace std::chrono;
    return duration_cast<duration<double> > (steady_clock::now().time_since_epoch()).count();
}

// Mutex-based wait function
void mutex_wait(std::atomic<int>& futex_var, int expected) {
    pthread_mutex_lock(&mutex);
    (void)futex_var;
    while (futex_var.load(std::memory_order_acquire) == expected) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

// Mutex-based wake function
void mutex_wake(std::atomic<int>& futex_var) {
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

// Logs client messages with mutex synchronization
void LagCompensation::logClientMessage(int client_fd, const std::string& message) {
    static std::atomic<int> futex_var(0);
    
    while (futex_var.exchange(1, std::memory_order_acquire) != 0) {
        mutex_wait(futex_var, 1);
    }
    
    double timestamp = getCurrentTime();
    TickRecord record;
    record.client_fd = client_fd;
    record.message = message;
    record.timestamp = timestamp;
    
    message_logs[client_fd].push_back(record);
    
    clearOldRecords();
    
    futex_var.store(0, std::memory_order_release);
    mutex_wake(futex_var);
}

// Retrieves valid messages (only those within 200ms)
std::deque<TickRecord> LagCompensation::getValidMessages(int client_fd) {
    static std::atomic<int> futex_var(0);
    
    while (futex_var.exchange(1, std::memory_order_acquire) != 0) {
        mutex_wait(futex_var, 1);
    }
    
    std::deque<TickRecord> valid_msgs;
    double current_time = getCurrentTime();
    
    if (message_logs.find(client_fd) == message_logs.end()) {
        futex_var.store(0, std::memory_order_release);
        mutex_wake(futex_var);
        return valid_msgs;
    }
    
    for (const TickRecord& record : message_logs[client_fd]) {
        if (current_time - record.timestamp < 0.2) {
            valid_msgs.push_back(record);
        }
    }
    
    futex_var.store(0, std::memory_order_release);
    mutex_wake(futex_var);
    return valid_msgs;
}

// Removes old records older than 1 second
void LagCompensation::clearOldRecords() {
    static std::atomic<int> futex_var(0);
    
    while (futex_var.exchange(1, std::memory_order_acquire) != 0) {
        mutex_wait(futex_var, 1);
    }
    
    double current_time = getCurrentTime();
    for (std::map<int, std::deque<TickRecord> >::iterator it = message_logs.begin(); it != message_logs.end(); ++it) {
        std::deque<TickRecord>& logs = it->second;
        while (!logs.empty() && current_time - logs.front().timestamp > 1.0) {
            logs.pop_front();
        }
    }
    
    futex_var.store(0, std::memory_order_release);
    mutex_wake(futex_var);
}

// Processes client messages with lag compensation and sends them to the server
void LagCompensation::processClientMessage(Server* server, int client_fd, const std::string& message) {
    logClientMessage(client_fd, message);
    
    // Retrieve valid messages from the buffer
    std::deque<TickRecord> valid_msgs = getValidMessages(client_fd);
    
    for (const TickRecord& record : valid_msgs) {
        server->parseMessage(client_fd, record.message);
    }
}
