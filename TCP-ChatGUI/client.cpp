#include "client.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

// Connect to server
int connectToServer(const std::string& ip, int port)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << "WSAStartup failed\n";
        return -1;
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed\n";
        return -1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

    if (connect(clientSocket, (sockaddr*)&server, sizeof(server)) < 0)
    {
        std::cout << "Connection failed\n";
        closesocket(clientSocket);
        return -1;
    }

    return clientSocket;
}

// Receive messages
void receiveMessages(int clientSocket, std::function<void(std::string)> callback)
{
    char buffer[1024];

    while (true)
    {
        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytes <= 0)
            break;

        std::string msg(buffer, bytes);

        // Send message to GUI
        callback(msg);
    }
}