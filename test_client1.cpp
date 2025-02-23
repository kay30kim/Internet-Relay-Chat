#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 6667  // Server port
#define BUFFER_SIZE 1024

int main() {
    // 1. Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    // 2. Set up server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // 3. Connect to server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to connect to the server" << std::endl;
        return 1;
    }

    std::cout << "Connected to the server" << std::endl;

    // 4. Send CAP LS, NICK, and USER messages
    std::string cap_ls_cmd = "CAP LS\r\n";  // First message for server compatibility
    std::string nickname = "kay";
    std::string username = "Kyungho";
    std::string realname = "Kyungho Kim";

    std::string nick_cmd = "NICK " + nickname + "\r\n";
    std::string user_cmd = "USER " + username + " 0 * :" + realname + "\r\n";

    send(sock, cap_ls_cmd.c_str(), cap_ls_cmd.length(), 0);
    send(sock, nick_cmd.c_str(), nick_cmd.length(), 0);
    send(sock, user_cmd.c_str(), user_cmd.length(), 0);

    std::cout << "Sent: " << cap_ls_cmd;
    std::cout << "Sent: " << nick_cmd;
    std::cout << "Sent: " << user_cmd;

    // 5. Receive server response
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        std::cout << "Server response: " << buffer << std::endl;
    }

    // 6. Keep sending messages
    std::string userInput;
    while (true) {
        std::cout << "Enter message ('exit' to quit): ";
        std::getline(std::cin, userInput);

        if (userInput == "exit") {
            std::cout << "Closing connection..." << std::endl;
            break;
        }

        userInput += "\r\n";  // Ensure proper IRC format
        send(sock, userInput.c_str(), userInput.length(), 0);
        std::cout << "Sent: " << userInput;

        // Receive response from the server
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::cout << "Server response: " << buffer << std::endl;
        }
    }

    // 7. Close the socket
    close(sock);
    return 0;
}
