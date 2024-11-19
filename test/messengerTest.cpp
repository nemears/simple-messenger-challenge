#include "simpleMessenger.h"
#include "gtest/gtest.h"

using namespace SimpleMessenger;

TEST(MessengerTests, shutdownTest) {
    Server server;
    Client client("127.0.0.1");
}

// TEST(MessengerTests, serverShutdownTest) {
//     Server server;
//     Client client("127.0.0.1");
//     server.shutdown();
//     client.shutdown();
// }

std::atomic_bool receivedMessage = false;

TEST(MessengerTests, sendMessage) {
    Server server;
    Client client("127.0.0.1");
    client.onMessage([](Message& message) {
        receivedMessage = true;
    });
    Message message = Message::from("hello!");
    server.send(message);
    ASSERT_TRUE(receivedMessage) << "Failed to receive message from server!";
    // client.shutdown();
    // server.shutdown();
}

TEST(MessengerTests, sendDataRace) {
    Server server;
    Client client("127.0.0.1");
    auto printMessage = [](Message& message) {
        std::cout << message.string() << std::endl;
    };
    server.onMessage(printMessage);
    client.onMessage(printMessage);
    std::thread clientProcess([&client]{
        Message message = Message::from("data");
        client.send(message);
    });
    Message message = Message::from("race");
    server.send(message);
    clientProcess.join();
    // client.shutdown();
    // server.shutdown();
}