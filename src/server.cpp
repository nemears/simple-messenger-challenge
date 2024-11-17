#include "server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>

using namespace Messenger;

Server::Server() {
    std::cout << "server initializing!" << std::endl; 

    // startup server
    // get address info
    struct addrinfo hints;
    struct addrinfo* address;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in IP
    if (getaddrinfo(0, m_port.c_str(), &hints, &address) != 0) {
        throw InitializationError();        
    }

    // get socket
    m_serverSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (m_serverSocket == -1) {
        throw InitializationError();
    }

    // bind to socket
    if (bind(m_serverSocket , address->ai_addr, address->ai_addrlen) < 0) {
        throw InitializationError();
    }

    // listen for connections
    if (::listen(m_serverSocket, 10) < 0) {
        throw InitializationError();
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
            log("error accepting client shutting down!");
            m_shuttingDown = true;
            return;
        }

        // set the socket
        setSocket(m_client);

        // handle messages coming in
        listen();
    }
}

void Server::shutdown() {
    SocketIO::shutdown();
    std::lock_guard<std::mutex> shuttingDownLock(m_shutingDownMtx);
    m_shuttingDown = true;
    close(m_serverSocket);
    m_shuttingDownCv.notify_all();
}

void Server::waitForShutdown() {
    std::unique_lock<std::mutex> shuttingDownLock(m_shutingDownMtx);
    m_shuttingDownCv.wait(shuttingDownLock, [this] {
        return m_shuttingDown;        
    });
}

const std::string noClientError = "server has no client to communicate with!";

void Server::send(Message& message) {
    if (m_client == -1) {
        throw MessengerError(noClientError);
    }

    SocketIO::send(message);
}
