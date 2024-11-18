#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include "message.h"

namespace  SimpleMessenger {
    class Messenger {
        private:
            int m_socket = -1;
            std::mutex m_socketMutex;
            std::condition_variable m_socketCv;
            bool m_awaitingResponse = false;
            bool m_waitingOnSend = false;
            bool m_shuttingDown = false;
            struct AbstractFunctor {
                virtual ~AbstractFunctor() {}
                virtual void operator()(Message& message) const = 0;
            };
            std::unique_ptr<AbstractFunctor> m_onMessageHandler;
        public:
            virtual ~Messenger() {}
            // send a message
            virtual void send(Message& message);
            // listen for messages until socket is closed
            void listen();
            // handle received messages
            template <typename F> // F is a functor or lambda expression
            void onMessage(F function) {
                 struct FunctorImpl : public AbstractFunctor {
                     F m_function;
                     FunctorImpl(F f) : m_function(f) {};
                     void operator()(Message& message) const {
                         m_function(message);
                     }
                 };
                 m_onMessageHandler = std::make_unique<FunctorImpl>(function);
            }
            // set socket (socket must already be closed!)
            void setSocket(int socket) {
                std::lock_guard<std::mutex> socketLock(m_socketMutex);
                m_socket = socket;
            }
            // close the socket (will terminate a listen call)
            virtual void shutdown();
    };
}
