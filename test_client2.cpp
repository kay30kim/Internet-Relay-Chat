#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>

#define PORT 6667
#define BUFFER_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to connect to the server" << std::endl;
        return 1;
    }

    std::cout << "Client2 connected to the server" << std::endl;

    std::string cap_ls_cmd = "CAP LS\r\n";
    std::string nick_cmd = "NICK kay2\r\n";
    std::string user_cmd = "USER kyungho2 0 * :Kyungho Kim2\r\n";

    send(sock, nick_cmd.c_str(), nick_cmd.length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    send(sock, user_cmd.c_str(), user_cmd.length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    char buffer[BUFFER_SIZE] = {0};

    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        std::cout << "Server response: " << buffer << std::endl;
    } else {
        std::cerr << "No initial response from server. Exiting..." << std::endl;
        close(sock);
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        std::string message = "PRIVMSG kay1 :Hello from Client2 - Message " + std::to_string(i + 1) + "\r\n";
        send(sock, message.c_str(), message.length(), 0);
        std::cout << "Client2 Sent: " << message;

        // 서버 응답 수신
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::cout << "Client2 received: " << buffer << std::endl;
        } else {
            std::cout << "No response from server, continuing..." << std::endl;
        }

        std::string ping_cmd = "PING :keepalive\r\n";
        send(sock, ping_cmd.c_str(), ping_cmd.length(), 0);
        std::cout << "Client2 Sent: " << ping_cmd;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(sock);
    return 0;
}
