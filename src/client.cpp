#include "client.h"
#include <iostream>

using namespace Messenger;

Client::Client(std::string& serverAddress) : m_serverAddress(serverAddress) {
    std::cout << "Client initializing!" << std::endl;
}
