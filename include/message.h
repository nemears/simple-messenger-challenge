#pragma once

#include <cstdint>
#include <exception>
#include <vector>
#include <string>

namespace Messenger {
    struct Message {
        std::vector<uint8_t> bytes;
        std::string string() const {
            return std::string(bytes.begin(), bytes.end()); 
        }
    };

    class MessengerError : public std::exception {
        private:
           const std::string m_msg = "ERROR!";
        public:
           MessengerError() {};
           MessengerError(std::string errorMessage) : m_msg(errorMessage) {};
           const char* what() const noexcept override {
                return m_msg.c_str();
           }
    };
}
