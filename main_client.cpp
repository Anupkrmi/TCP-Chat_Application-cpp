#include <iostream>
#include <winsock2.h>
#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <map>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// ---- Chat Storage ----
vector<string> groupMessages;
map<string, vector<string>> privateChats;

string activeChat = "group";

// ---- Redraw UI ----
void redrawScreen() {
    system("cls");

    cout << "========== GROUP CHAT ==========\n";
    for (auto &m : groupMessages) {
        cout << m << endl;
    }

    for (auto &chat : privateChats) {
        cout << "\n========== PRIVATE (" << chat.first << ") ==========\n";
        for (auto &m : chat.second) {
            cout << m << endl;
        }
    }

    cout << "\n[ACTIVE: " << activeChat << "]\n";
    cout << "You: ";
    cout.flush();
}

// ---- Extract sender from [PM:User] ----
string extractSender(const string& msg) {
    int start = msg.find("[PM:");
    int end = msg.find("]", start);

    if (start != string::npos && end != string::npos) {
        return msg.substr(start + 4, end - (start + 4));
    }
    return "";
}

// ---- Receive Messages ----
void receiveMessages(SOCKET clientSocket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "\nDisconnected from server.\n";
            break;
        }

        string msg = buffer;

        // ---- Private Message ----
        if (msg.find("[PM:") != string::npos) {
            string sender = extractSender(msg);
            privateChats[sender].push_back(msg);

            if (privateChats[sender].size() > 10)
                privateChats[sender].erase(privateChats[sender].begin());
        }
        else {
            groupMessages.push_back(msg);

            if (groupMessages.size() > 10)
                groupMessages.erase(groupMessages.begin());
        }

        redrawScreen();
    }
}

// ---- Send Messages ----
void sendMessages(SOCKET clientSocket) {
    while (true) {
        string message;

        getline(cin, message);

        if (message == "/exit") {
            break;
        }

        // ---- Switch focus ----
        if (message.rfind("/focus ", 0) == 0) {
            string user = message.substr(7);

            if (user.empty()) {
                cout << "Usage: /focus <username>\n";
                continue;
            }

            activeChat = user;
            redrawScreen();
            continue;
        }

        // ---- Group chat ----
        if (activeChat == "group") {
            send(clientSocket, message.c_str(), message.length(), 0);
        }
        else {
            // ---- Private chat ----
            string fullMsg = "/msg " + activeChat + " " + message;

            // Add to local view
            privateChats[activeChat].push_back("[You -> " + activeChat + "] " + message);

            if (privateChats[activeChat].size() > 10)
                privateChats[activeChat].erase(privateChats[activeChat].begin());

            send(clientSocket, fullMsg.c_str(), fullMsg.length(), 0);
        }

        redrawScreen();
    }
}

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    WSAStartup(MAKEWORD(2,2), &wsa);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Connection failed\n";
        return 1;
    }

    cout << "Connected to server!\n";

    // ---- Username ----
    string username;
    cout << "Enter your username: ";
    getline(cin, username);

    send(clientSocket, username.c_str(), username.length(), 0);

    // ---- Threads ----
    thread recvThread(receiveMessages, clientSocket);
    thread sendThread(sendMessages, clientSocket);

    recvThread.join();
    sendThread.join();

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}