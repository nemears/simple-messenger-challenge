#pragma once

#include "socketIO.h"
#include <thread>

namespace Messenger {
    class Client : public SocketIO {
        private:
            std::thread m_listenProcess;
        public:
            Client(std::string& serverAddress);
            void shutdown() override;
    };
}
