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

    std::cout << "Client1 connected to the server" << std::endl;

    std::string cap_ls_cmd = "CAP LS\r\n";
    std::string nick_cmd = "NICK kay1\r\n";
    std::string user_cmd = "USER kyungho1 0 * :Kyungho Kim1\r\n";

    send(sock, cap_ls_cmd.c_str(), cap_ls_cmd.length(), 0);
    send(sock, nick_cmd.c_str(), nick_cmd.length(), 0);
    send(sock, user_cmd.c_str(), user_cmd.length(), 0);

    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        std::cout << "Server response: " << buffer << std::endl;
    }

    for (int i = 0; i < 30; i++) {
        std::string message = "PRIVMSG kay2 :Hello from Client1 - Message " + std::to_string(i+1) + "\r\n";
        send(sock, message.c_str(), message.length(), 0);
        std::cout << "Client1 Sent: " << message;

        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::cout << "Client1 received: " << buffer << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(sock);
    return 0;
}
