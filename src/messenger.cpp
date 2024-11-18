#include "messenger.h"
#include <mutex>
#include <netinet/in.h>
#include <poll.h>

using namespace SimpleMessenger;

const std::string sendErrorMessage = "could not send message!";

void Messenger::send(Message& message) {
    // lock mutual exclusion for scope
    std::unique_lock<std::mutex> socketLock(m_socketMutex);

    // protocol specifies to send number of bytes in a long first
    // and then read that many bytes into the message buffer
    uint32_t messageSize = message.bytes.size();
    uint32_t messageSizeTranslation = htonl(messageSize);
    auto uint32_tSize = sizeof(uint32_t);
    m_awaitingResponse = true;
    int sendResult = ::send(m_socket, &messageSizeTranslation, uint32_tSize, 0);
    if (sendResult != uint32_tSize) {
        throw MessengerError(sendErrorMessage);
    }
    std::size_t bytesSent = 0;
    while (bytesSent < messageSize) {
        sendResult = ::send(m_socket, &message.bytes[bytesSent], messageSize - bytesSent, 0);
        if (sendResult == -1) {
            throw MessengerError(sendErrorMessage);
        }
        bytesSent += sendResult;
    }

    // make sure listen is in right state
    m_socketCv.wait(socketLock, [this] {
        return m_waitingOnSend;        
    });
    m_waitingOnSend = false;

    // wait for response from client
    uint8_t responseBuffer = 0;
    int receiveResult = ::recv(m_socket, &responseBuffer, sizeof(uint8_t), 0);
    if (receiveResult == -1) {
        throw MessengerError(sendErrorMessage);
    }
    m_awaitingResponse = false;
    socketLock.unlock();
    m_socketCv.notify_one();
}

const std::string recvErrorMessage = "could not receive mesage!";

void Messenger::listen() {
    // receive messages
    struct pollfd pfds[1];
    pfds[0].fd = m_socket;
    pfds[0].events = POLLIN;// | POLLERR | POLLHUP;

    // message loop
    for(;;) {
        // poll lets us know if there is data to be read
        // or if the socket has been hungup or errored
        int pollResult = poll(pfds, 1, -1);
        if (pollResult == 0) {
            // nothing polled, weird
            continue;
        }
        if (pfds[0].revents & POLLIN) {
            // data ready to read
            if (m_awaitingResponse) {
                // let send know we are in state to skip recv and wait for signal from send
                m_waitingOnSend = true;
                m_socketCv.notify_one();

                // data being read is response to send call,
                // wait for send to finish it's operation
                std::unique_lock<std::mutex> clientLock(m_socketMutex);
                m_socketCv.wait(clientLock, [this] {
                    return !m_awaitingResponse;        
                });
            } else {
                // read message into object
                Message message;
                {
                    // lock the socket for this scope while we are receiving
                    std::lock_guard<std::mutex> clientLock(m_socketMutex);

                    // get size of message
                    uint32_t messageSize = 0;
                    auto uint32_tSize = sizeof(uint32_t);
                    int recvResult = ::recv(m_socket, &messageSize, uint32_tSize, 0);
                    if (recvResult == -1) {
                        throw MessengerError(recvErrorMessage);
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
                            throw MessengerError(recvErrorMessage);
                        }
                        bytesRead += recvResult;
                    }

                    // send aknowledgement
                    messageSize = htonl(messageSize);
                    int sendResult = ::send(m_socket, &messageSize, uint32_tSize, 0);
                    if (sendResult == -1) {
                        throw MessengerError(recvErrorMessage);
                    
                    }
                    message = Message{ buffer };
                }

                // run user defined handlers
                if (m_onMessageHandler) {
                    (*m_onMessageHandler)(message);
                }
            }
        } else {
            // connection is errored or closed, stop processing
            break;
        }
    }
}

void Messenger::shutdown() {
    std::lock_guard<std::mutex> socketLock(m_socketMutex);
    close(m_socket);
    m_socket = -1;
}
