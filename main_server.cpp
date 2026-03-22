#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

/*
    Store clients as:
    SOCKET + username
*/
vector<pair<SOCKET, string>> clients;
mutex clientsMutex;

/*
    Broadcast message to all clients except sender
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
    Send private message to a specific user
*/
void sendPrivateMessage(const string& targetUser, const string& message) {
    lock_guard<mutex> lock(clientsMutex);

    for (auto &client : clients) {
        if (client.second == targetUser) {
            send(client.first, message.c_str(), message.length(), 0);
            return;
        }
    }
}

/*
    Send list of online users to requesting client
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
    Handle each client connection
*/
void handleClient(SOCKET clientSocket) {
    char buffer[1024];

    // ---- Step 1: Receive username ----
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);

    string username = buffer;

    {
        lock_guard<mutex> lock(clientsMutex);
        clients.push_back({clientSocket, username});
    }

    cout << username << " joined the chat\n";
    broadcastMessage(username + " joined the chat", clientSocket);

    // ---- Step 2: Chat loop ----
    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << username << " left the chat\n";
            broadcastMessage(username + " left the chat", clientSocket);
            break;
        }

        string message = buffer;

        // ---- Handle commands ----

        // Private message: /msg username message
        if (message.rfind("/msg ", 0) == 0) {
            int firstSpace = message.find(' ', 5);

            if (firstSpace != string::npos) {
                string targetUser = message.substr(5, firstSpace - 5);
                string privateMsg = message.substr(firstSpace + 1);

                string fullMsg = "[PRIVATE] " + username + ": " + privateMsg;

                sendPrivateMessage(targetUser, fullMsg);
            }
            continue;
        }

        // User list command
        if (message == "/list") {
            sendUserList(clientSocket);
            continue;
        }

        // ---- Normal message ----
        cout << message << endl;
        broadcastMessage(message, clientSocket);
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