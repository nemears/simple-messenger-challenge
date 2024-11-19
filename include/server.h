#pragma once

#include <string>
#include <thread>
#include "messenger.h"

#ifndef SERVER_PORT
#define SERVER_PORT 1997
#endif

namespace SimpleMessenger {
    class Server : public Messenger {
        private:
            const std::string m_port = std::to_string(SERVER_PORT);
            int m_serverSocket = -1;
            std::thread m_singleClientProcess;
            bool m_shuttingDown = false;
            int m_client = -1;

            void singleClient();
        public:
            Server();
            virtual ~Server() {
                if (!m_shuttingDown) {
                    shutdown();
                }
            }
            void shutdown() override;
            void send(Message& msg) override;
    };
}
