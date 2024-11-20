#include "messenger.h"
#include <netinet/in.h>
#include <cerrno>
#include <cstring>

#define SHUTDOWN_SIGNAL 0
#define MESSAGE_SIGNAL 1
#define AKNOWLEDGEMENT_SIGNAL 2

using namespace SimpleMessenger;

const std::string sendErrorMessage = "could not send message! ";

void Messenger::send(Message& message) {
    // lock mutual exclusion for scope
    std::lock_guard<std::mutex> sendLock(m_sendMutex);
    std::unique_lock<std::mutex> socketLock(m_socketMutex);

    // start message with signal showing this is a message
    uint8_t messageSignal = MESSAGE_SIGNAL;
    int sendResult = ::send(m_socket, &messageSignal, sizeof(uint8_t), 0);
    if (sendResult == -1 || sendResult == 0) {
        throw MessengerError(sendErrorMessage + std::strerror(errno));
    }

    // protocol specifies to send number of bytes in a long first
    // and then read that many bytes into the message buffer
    uint32_t messageSize = message.bytes.size();
    uint32_t messageSizeTranslation = htonl(messageSize);
    auto uint32_tSize = sizeof(uint32_t);
    sendResult = ::send(m_socket, &messageSizeTranslation, uint32_tSize, 0);
    if (sendResult != uint32_tSize) {
        throw MessengerError(sendErrorMessage + std::strerror(errno));
    }
    std::size_t bytesSent = 0;
    while (bytesSent < messageSize) {
        sendResult = ::send(m_socket, &message.bytes[bytesSent], messageSize - bytesSent, 0);
        if (sendResult == -1) {
            throw MessengerError(sendErrorMessage + std::strerror(errno));
        }
        bytesSent += sendResult;
    }

    // wait for aknowledgment
    m_socketCv.wait(socketLock, [this] {
        return m_messageReceived;
    });
    m_messageReceived = false;
}

const std::string recvErrorMessage = "could not receive mesage! ";

void Messenger::listen() {
    // receive messages

    for(;;) {
        uint8_t typeBit = 0;
        int recvResult = ::recv(m_socket, &typeBit, sizeof(uint8_t), 0);

        if (recvResult == -1) {
            throw MessengerError(recvErrorMessage + std::strerror(errno));
        } else if (recvResult == 0) {
            // other messenger closed connection
            break;
        }

        if (typeBit == SHUTDOWN_SIGNAL) {
            if (!m_shuttingDown) {
                // send signal back to confirm shutdown / trigger recv for other messenger
                ::send(m_socket, &typeBit, sizeof(uint8_t), 0);
            }

            // close socket, and exit listen loop
            close(m_socket);
            m_socket = -1;
            break;
        } else if (typeBit == MESSAGE_SIGNAL) {
            // read message into object
            // get size of message
            uint32_t messageSize = 0;
            auto uint32_tSize = sizeof(uint32_t);
            int recvResult = ::recv(m_socket, &messageSize, uint32_tSize, 0);
            if (recvResult == -1) {
                throw MessengerError(recvErrorMessage + std::strerror(errno));
            } else if (recvResult == 0) {
                // socket is closed
                break;
            }

            // run network number conversion just in case
            messageSize = ntohl(messageSize);

            // receive rest of message
            std::vector<uint8_t> buffer(messageSize);
            std::size_t bytesRead = 0;
            while (bytesRead < messageSize) {
                recvResult = ::recv(m_socket, &buffer[bytesRead], messageSize - bytesRead, 0);
                if (recvResult == -1) {
                    throw MessengerError(recvErrorMessage + std::strerror(errno));
                }
                bytesRead += recvResult;
            }

            Message message = Message{ buffer };

            // run user defined handlers
            if (m_onMessageHandler) {
                (*m_onMessageHandler)(message);
            }

            // send aknowledgement
            std::lock_guard<std::mutex> clientLock(m_socketMutex);
            uint8_t aknowledgmentSignal = AKNOWLEDGEMENT_SIGNAL;
            int sendResult = ::send(m_socket, &aknowledgmentSignal, sizeof(uint8_t), 0);
            if (sendResult == -1) {
                throw MessengerError(recvErrorMessage + std::strerror(errno));
            }
        } else if (typeBit == AKNOWLEDGEMENT_SIGNAL) {
            std::lock_guard<std::mutex> socketLck(m_socketMutex);
            m_messageReceived = true;
            m_socketCv.notify_one();
        } else {
            throw MessengerError("Bad state got bad signal from other messenger!");
        }
    }
}

void Messenger::shutdown() {
    if (m_shuttingDown) {
        return;
    }
    std::lock_guard<std::mutex> socketLock(m_socketMutex);
    m_shuttingDown = true;
    uint8_t shutdownSignal = SHUTDOWN_SIGNAL;
    ::send(m_socket, &shutdownSignal, sizeof(uint8_t), 0);
}
