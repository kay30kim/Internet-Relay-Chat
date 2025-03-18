#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

// Server Configuration
const string SERVER_IP = "127.0.0.1"; // Replace with actual IRC server IP
const int SERVER_PORT = 6667;
const string NICKNAME = "MyClient";  // Change as needed
const string CHANNEL = "#mychannel"; // Default channel to join

// Global socket for communication
int clientSocket;

// Function to receive messages from the server
void receiveMessages() {
    char buffer[2048];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cout << "Disconnected from server." << endl;
            close(clientSocket);
            exit(0);
        }
        string message(buffer);
        
        // Handling PING message from the server
        if (message.find("PING") != string::npos) {
            string response = "PONG " + message.substr(5) + "\r\n";
            send(clientSocket, response.c_str(), response.length(), 0);
        }

        cout << message;
    }
}

// Function to send commands to the server
void sendCommand(const string& command) {
    string msg = command + "\r\n";
    send(clientSocket, msg.c_str(), msg.length(), 0);
}

int main() {
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Socket creation failed." << endl;
        return -1;
    }

    // Server address setup
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Connection failed." << endl;
        return -1;
    }

    cout << "Connected to IRC server." << endl;

    // Send IRC commands to register
    sendCommand("NICK " + NICKNAME);
    sendCommand("USER " + NICKNAME + " 0 * :" + NICKNAME);

    // Start receiving messages in a separate thread
    thread recvThread(receiveMessages);
    recvThread.detach();

    // Command loop
    while (true) {
        cout << "\n====== IRC Client Menu ======" << endl;
        cout << "1. JOIN Channel" << endl;
        cout << "2. INVITE User" << endl;
        cout << "3. KICK User" << endl;
        cout << "4. Send Message (PRIVMSG)" << endl;
        cout << "5. QUIT Server" << endl;
        cout << "6. WHO List" << endl;
        cout << "7. Change TOPIC" << endl;
        cout << "8. Set MODE" << endl;
        cout << "9. LIST Channels" << endl;
        cout << "10. Show NAMES" << endl;
        cout << "0. Exit" << endl;
        cout << "Select an option: ";
        
        int choice;
        cin >> choice;
        cin.ignore(); // Clear newline character

        string input, user, message;

        switch (choice) {
            case 1: // JOIN
                cout << "Enter channel to join: ";
                getline(cin, input);
                sendCommand("JOIN " + input);
                break;

            case 2: // INVITE
                cout << "Enter username to invite: ";
                getline(cin, user);
                sendCommand("INVITE " + user + " " + CHANNEL);
                break;

            case 3: // KICK
                cout << "Enter username to kick: ";
                getline(cin, user);
                cout << "Enter reason: ";
                getline(cin, message);
                sendCommand("KICK " + CHANNEL + " " + user + " :" + message);
                break;

            case 4: // PRIVMSG (Send Message)
                cout << "Enter recipient (user or channel): ";
                getline(cin, user);
                cout << "Enter message: ";
                getline(cin, message);
                sendCommand("PRIVMSG " + user + " :" + message);
                break;

            case 5: // QUIT
                cout << "Enter quit message: ";
                getline(cin, message);
                sendCommand("QUIT :" + message);
                close(clientSocket);
                return 0;

            case 6: // WHO
                sendCommand("WHO " + CHANNEL);
                break;

            case 7: // TOPIC
                cout << "Enter new topic: ";
                getline(cin, message);
                sendCommand("TOPIC " + CHANNEL + " :" + message);
                break;

            case 8: // MODE
                cout << "Enter mode command: ";
                getline(cin, message);
                sendCommand("MODE " + CHANNEL + " " + message);
                break;

            case 9: // LIST Channels
                sendCommand("LIST");
                break;

            case 10: // NAMES
                sendCommand("NAMES " + CHANNEL);
                break;

            case 0: // Exit
                cout << "Exiting program." << endl;
                close(clientSocket);
                return 0;

            default:
                cout << "Invalid selection. Try again." << endl;
        }
    }

    return 0;
}
