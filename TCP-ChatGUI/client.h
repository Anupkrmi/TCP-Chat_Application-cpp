#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <functional>

int connectToServer(const std::string& ip, int port);

void receiveMessages(int clientSocket, std::function<void(std::string)> callback);

#endif