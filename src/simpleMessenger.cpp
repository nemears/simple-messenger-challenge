#include "simpleMessenger.h"
#include <ctime>
#include <mutex>
#include <string>
#include <iostream>

namespace SimpleMessenger {
void simpleMessenger(Messenger& messenger) {
    // assuming messenger is already listening for messages in the background
    // this function will set onMessage and send messages from std::cin
    
    std::cout << "type \"quit\" to stop the program" << std::endl;

    std::mutex logMtx;
    messenger.onMessage([&logMtx](Message& message){
        std::lock_guard<std::mutex> logLck(logMtx);
        std::cout << "message received: " << message.string() << std::endl;
    });
    for (;;) {
        std::string buffer;
        std::getline(std::cin, buffer);
        if (buffer == "quit") {
            messenger.shutdown();
            std::lock_guard<std::mutex> logLck(logMtx);
            std::cout << "successfully shutdown" << std::endl;
            break;
        }
        std::lock_guard<std::mutex> logLck(logMtx);
        Message message  = Message::from(buffer);
        const std::clock_t sendStart = std::clock();
        messenger.send(message);
        const std::clock_t sendEnd = std::clock();
        std::cout << "sent in: " << 1000.0 * (sendEnd - sendStart) / CLOCKS_PER_SEC << "ms" << std::endl;
    }
}
}
