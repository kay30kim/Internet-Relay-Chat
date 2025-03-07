#include "LagCompensation.hpp"
#include <chrono>

LagCompensation& LagCompensation::getInstance() {
    static LagCompensation instance;
    return instance;
}

double LagCompensation::getCurrentTime() {
    using namespace std::chrono;
    return duration_cast<duration<double> >(steady_clock::now().time_since_epoch()).count(); // `>>` 간격 추가
}

void LagCompensation::logClientMessage(int client_fd, const std::string& message) {
    double timestamp = getCurrentTime();
    
    // C++98 호환성을 위해 TickRecord 객체를 명시적으로 생성 후 push_back
    TickRecord record;
    record.client_fd = client_fd;
    record.message = message;
    record.timestamp = timestamp;
    
    message_logs[client_fd].push_back(record);  // 중괄호 초기화 대신 명시적 객체 사용

    // 오래된 기록 제거
    clearOldRecords();
}

std::deque<TickRecord> LagCompensation::getValidMessages(int client_fd) {
    std::deque<TickRecord> valid_msgs;
    double current_time = getCurrentTime();
    
    for (std::deque<TickRecord>::iterator it = message_logs[client_fd].begin();
         it != message_logs[client_fd].end(); ++it) {
        if (current_time - it->timestamp < 0.2) {  // 200ms 이내 데이터만 유효
            valid_msgs.push_back(*it);
        }
    }
    
    return valid_msgs;
}

void LagCompensation::clearOldRecords() {
    double current_time = getCurrentTime();
    
    for (std::map<int, std::deque<TickRecord> >::iterator it = message_logs.begin();
         it != message_logs.end(); ++it) {
        std::deque<TickRecord>& logs = it->second;

        while (!logs.empty() && current_time - logs.front().timestamp > 1.0) {
            logs.pop_front();
        }
    }
}
