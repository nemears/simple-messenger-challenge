#pragma once

#include <condition_variable>
#include <exception>
#include <mutex>
#include <string>
#include <thread>
#include <iostream>
#include "socketIO.h"

#ifndef SERVER_PORT
#define SERVER_PORT 1997
#endif

namespace Messenger {
    class Server : public SocketIO {
        private:
            const std::string m_port = std::to_string(SERVER_PORT);
            int m_serverSocket = -1;
            std::thread m_singleClientProcess;
            bool m_shuttingDown = false;
            std::mutex m_shutingDownMtx;
            std::condition_variable m_shuttingDownCv;
            int m_client = -1;
            std::mutex m_logMtx;

            void singleClient();
        public:
            Server();
            struct InitializationError : public std::exception {
                const char* what() const noexcept override {
                    return "Could not Initialize Server!";
                }
            };
            template <typename T>
            void log(T& message) {
                std::lock_guard<std::mutex> logLock(m_logMtx);
                std::cout << message << std::endl;
            }
            void shutdown() override;
            void waitForShutdown();
            void send(Message& msg) override;
    };
}
