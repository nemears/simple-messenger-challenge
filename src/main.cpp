#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include "client.h"
#include "server.h"

using namespace Messenger;

int main (int argc, char* argv[]) {
    std::optional<std::string> firstArgument = std::nullopt;
    if (argc >= 2) {
        firstArgument = std::optional<std::string>{argv[1]};
    }
    
    try {
        if (firstArgument) {
            std::cout << "Connecting to simple-messenger server at address: " << *firstArgument << std::endl;
            Client client(firstArgument.value());
        } else {
            std::cout << "Running Server on local port " << SERVER_PORT << std::endl;
            Server server;
            server.waitForShutdown();
        }
    } catch (std::exception& e) {
        std::cout << e.what();
        return 1;
    }
    return 0;
}
