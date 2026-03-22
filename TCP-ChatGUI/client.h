#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <functional>

// Connect to server
int connectToServer(const std::string& ip, int port);

// Receive messages using callback
void receiveMessages(int clientSocket, std::function<void(std::string)> callback);

#endif