#pragma once

#include "messenger.h"
#include <thread>

namespace SimpleMessenger {
    class Client : public Messenger {
        private:
            std::thread m_listenProcess;
        public:
            virtual ~Client();
            Client(std::string serverAddress);
    };
}
