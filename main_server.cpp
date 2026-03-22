#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Store (socket, username)
vector<pair<SOCKET, string>> clients;
mutex clientsMutex;

/*
    Get current time [HH:MM]
*/
string getCurrentTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    stringstream ss;
    ss << "["
       << setw(2) << setfill('0') << ltm->tm_hour << ":"
       << setw(2) << setfill('0') << ltm->tm_min
       << "]";

    return ss.str();
}

/*
    Broadcast to all except sender
*/
void broadcastMessage(const string& message, SOCKET sender) {
    lock_guard<mutex> lock(clientsMutex);

    for (auto &client : clients) {
        if (client.first != sender) {
            send(client.first, message.c_str(), message.length(), 0);
        }
    }
}

/*
    Send private message
*/
bool sendPrivateMessage(const string& targetUser, const string& message) {
    lock_guard<mutex> lock(clientsMutex);

    for (auto &client : clients) {
        if (client.second == targetUser) {
            send(client.first, message.c_str(), message.length(), 0);
            return true;
        }
    }

    return false;
}

/*
    Send list of users
*/
void sendUserList(SOCKET clientSocket) {
    lock_guard<mutex> lock(clientsMutex);

    string list = "Online Users:\n";

    for (auto &client : clients) {
        list += "- " + client.second + "\n";
    }

    send(clientSocket, list.c_str(), list.length(), 0);
}

/*
    Send help menu
*/
void sendHelp(SOCKET clientSocket) {
    string help =
        "========== HELP ==========\n"
        "Messaging:\n"
        "  /msg <user> <message>\n\n"
        "Navigation:\n"
        "  /focus <user> (client-side)\n\n"
        "General:\n"
        "  /list\n"
        "  /help\n"
        "  /exit\n"
        "==========================\n";

    send(clientSocket, help.c_str(), help.length(), 0);
}

/*
    Handle each client
*/
void handleClient(SOCKET clientSocket) {
    char buffer[1024];

    // ---- Receive username ----
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);

    string username = buffer;

    // ---- Duplicate username check ----
    {
        lock_guard<mutex> lock(clientsMutex);

        for (auto &client : clients) {
            if (client.second == username) {
                string msg = "Username already taken. Disconnecting.";
                send(clientSocket, msg.c_str(), msg.length(), 0);
                closesocket(clientSocket);
                return;
            }
        }

        clients.push_back({clientSocket, username});
    }

    // ---- Join message ----
    string joinMsg = getCurrentTime() + " " + username + " joined (" 
                   + to_string(clients.size()) + " users online)";
    cout << joinMsg << endl;
    broadcastMessage(joinMsg, clientSocket);

    // ---- Chat loop ----
    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            string leaveMsg = getCurrentTime() + " " + username + " left the chat";
            cout << leaveMsg << endl;
            broadcastMessage(leaveMsg, clientSocket);
            break;
        }

        string message = buffer;

        // ---- /help ----
        if (message == "/help") {
            sendHelp(clientSocket);
            continue;
        }

        // ---- /list ----
        if (message == "/list") {
            sendUserList(clientSocket);
            continue;
        }

        // ---- /msg ----
        if (message.rfind("/msg ", 0) == 0) {
            int firstSpace = message.find(' ', 5);

            if (firstSpace == string::npos) {
                string error = "Usage: /msg <user> <message>";
                send(clientSocket, error.c_str(), error.length(), 0);
                continue;
            }

            string targetUser = message.substr(5, firstSpace - 5);
            string privateMsg = message.substr(firstSpace + 1);

            if (privateMsg.empty()) {
                string error = "Message cannot be empty.";
                send(clientSocket, error.c_str(), error.length(), 0);
                continue;
            }

            // ---- Format: [PM:username] ----
            string fullMsg = getCurrentTime() + " [PM:" + username + "] " + privateMsg;

            bool sent = sendPrivateMessage(targetUser, fullMsg);

            if (!sent) {
                string error = "User not found: " + targetUser;
                send(clientSocket, error.c_str(), error.length(), 0);
            }

            continue;
        }

        // ---- Unknown command ----
        if (!message.empty() && message[0] == '/') {
            string error = "Unknown command. Type /help";
            send(clientSocket, error.c_str(), error.length(), 0);
            continue;
        }

        // ---- Normal message ----
        string timedMessage = getCurrentTime() + " " + username + ": " + message;

        cout << timedMessage << endl;
        broadcastMessage(timedMessage, clientSocket);
    }

    // ---- Remove client ----
    {
        lock_guard<mutex> lock(clientsMutex);
        clients.erase(remove_if(clients.begin(), clients.end(),
            [clientSocket](auto &client) {
                return client.first == clientSocket;
            }), clients.end());
    }

    closesocket(clientSocket);
}

/*
    Main server
*/
int main() {
    WSADATA wsa;
    SOCKET serverSocket;
    sockaddr_in serverAddr;

    WSAStartup(MAKEWORD(2,2), &wsa);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    cout << "Server started...\n";

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        cout << "New client connected!\n";

        thread(handleClient, clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}