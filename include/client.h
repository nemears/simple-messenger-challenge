#pragma once

#include "messenger.h"
#include <thread>

namespace SimpleMessenger {
    class Client : public Messenger {
        private:
            std::thread m_listenProcess;
        public:
            Client(std::string& serverAddress);
            void shutdown() override;
    };
}
