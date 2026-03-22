#include <iostream>
#include <winsock2.h>
#include <thread>
#include <string>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

/*
    Receive messages from server
*/
void receiveMessages(SOCKET clientSocket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "\nDisconnected from server.\n";
            break;
        }

        // Clear current line and print incoming message
        cout << "\r" << buffer << endl;

        // Reprint prompt
        cout << "You: ";
        cout.flush();
    }
}

/*
    Send messages (with private chat mode support)
*/
void sendMessages(SOCKET clientSocket) {
    string activeChatUser = "";

    while (true) {
        string message;

        cout << "You: ";
        getline(cin, message);

        // ---- Exit private chat ----
        if (message == "/exit") {
            if (activeChatUser != "") {
                cout << "Exited private chat with " << activeChatUser << endl;
                activeChatUser = "";
                continue;
            } else {
                break;
            }
        }

        // ---- Enter private chat ----
        if (message.rfind("/chat ", 0) == 0) {
            activeChatUser = message.substr(6);
            cout << "[Private chat with " << activeChatUser << "]\n";
            continue;
        }

        // ---- If in private chat ----
        if (activeChatUser != "") {
            string fullMsg = "/msg " + activeChatUser + " " + message;
            send(clientSocket, fullMsg.c_str(), fullMsg.length(), 0);
            continue;
        }

        // ---- Normal message ----
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }

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