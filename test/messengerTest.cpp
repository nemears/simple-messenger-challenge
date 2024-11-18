#include "simpleMessenger.h"
#include "gtest/gtest.h"

using namespace SimpleMessenger;

TEST(MessengerTests, shutdownTest) {
    Server server;
    Client client("127.0.0.1");
    client.shutdown();
    server.shutdown();
}

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
    client.shutdown();
    server.shutdown();
}