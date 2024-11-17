#pragma once

#include <string>

namespace Messenger {
    class Client {
        private:
            std::string m_serverAddress = "";
        public:
            Client(std::string& serverAddress);
    };
}
