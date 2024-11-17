cmake_minimum_required(VERSION 3.30)
project(ahrefs-simple-messenger VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 20) 

#linting
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# Headers
include_directories(include)

# library
add_library(messenger src/server.cpp src/client.cpp src/messenger.cpp src/simpleMessenger.cpp)

# executable
add_executable(simple-messenger src/main.cpp)
target_link_libraries(simple-messenger PUBLIC messenger)
target_link_libraries(simple-messenger PRIVATE -fsanitize=address)
