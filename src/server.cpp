#include "server.h"
#include "message.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>

using namespace SimpleMessenger;

const std::string initializationMessage = "Could not initialize server, internal socket error!";

Server::Server() {
    // startup server
    // get address info
    addrinfo hints;
    addrinfo* address;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in IP
    if (getaddrinfo(0, m_port.c_str(), &hints, &address) != 0) {
        throw MessengerError(initializationMessage);
    }

    // get socket
    m_serverSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (m_serverSocket == -1) {
        freeaddrinfo(address);
        throw MessengerError(initializationMessage);
    }

    // bind to socket
    if (bind(m_serverSocket , address->ai_addr, address->ai_addrlen) < 0) {
        freeaddrinfo(address);
        throw MessengerError(initializationMessage);
    }

    // listen for connections
    if (::listen(m_serverSocket, 10) < 0) {
        freeaddrinfo(address);
        throw MessengerError(initializationMessage);
    }

    // cleanup address
    freeaddrinfo(address);

    // accept clients
    m_singleClientProcess = std::thread(&Server::singleClient, this);
}

void Server::singleClient() {
    while (!m_shuttingDown) {
        // accept client socket
        struct addrinfo* clientAddress;
        socklen_t addrSize = sizeof clientAddress;
        m_client = accept(m_serverSocket, (struct sockaddr *)&clientAddress, &addrSize);
        if (m_client == -1) {
            // log error and return
            m_shuttingDown = true;
            return;
        }

        if (m_shuttingDown) {
            // spoofed client
            close(m_serverSocket);
            break;
        }

        // send aknowledgement
        uint8_t initSignal = 0;
        if (::send(m_client, &initSignal, sizeof(uint8_t), 0) == -1) {
            throw MessengerError("Error aknowledging client acceptance");
        }

        // set the socket
        setSocket(m_client);

        // handle messages coming in
        listen();
        m_client = -1;
    }
}

const std::string spoofClientError = "Error on shutdown spoofing client";

void Server::shutdown() {
    Messenger::shutdown();

    m_shuttingDown = true;

    // send spoofed client
    addrinfo hints;
    addrinfo* address;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in IP
    if (getaddrinfo(0, m_port.c_str(), &hints, &address) != 0) {
        throw MessengerError(spoofClientError);
    }

    // get socket
    int clientSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (clientSocket == -1) {
        freeaddrinfo(address);
        throw MessengerError(spoofClientError);
    }

    // connect
    if (connect(clientSocket, address->ai_addr, address->ai_addrlen) == -1) {
        freeaddrinfo(address);
        throw MessengerError(spoofClientError);
    }

    freeaddrinfo(address);
    close(clientSocket);

    m_singleClientProcess.join();
}

const std::string noClientError = "server has no client to communicate with!";

void Server::send(Message& message) {
    if (m_client == -1) {
        throw MessengerError(noClientError);
    }

    Messenger::send(message);
}
