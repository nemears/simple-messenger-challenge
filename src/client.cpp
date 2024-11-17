#include "client.h"
#include "message.h"
#include "server.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <thread>

using namespace Messenger;

const std::string initializationError = "Internal error for client when initializing socket!";

Client::Client(std::string& serverAddress) {
    std::cout << "Client initializing!" << std::endl;

    // split serverAdress into host and port
    auto colonIndex = serverAddress.find(":");
    std::string host;
    std::string port = std::to_string(SERVER_PORT);
    if (colonIndex == std::string::npos) {
        host = serverAddress;
    } else {
        host = serverAddress.substr(0, colonIndex);
        port = serverAddress.substr(colonIndex + 1);
    }

    // connect to server
    addrinfo hints;
    addrinfo* address;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in IP
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &address) != 0) {
        throw MessengerError(initializationError);
    }

    // get socket
    int clientSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (clientSocket == -1) {
        throw MessengerError(initializationError);
    }

    // connect
    if (connect(clientSocket, address->ai_addr, address->ai_addrlen) == -1) {
        throw MessengerError(initializationError);
    }
   
    freeaddrinfo(address);

    // set socket for SocketIO
    setSocket(clientSocket);

    // start listening in background thread
    m_listenProcess = std::thread(&SocketIO::listen, this);
}

void Client::shutdown() {
    SocketIO::shutdown();
    m_listenProcess.join();
}
