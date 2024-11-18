cmake_minimum_required(VERSION 3.30)
project(ahrefs-simple-messenger VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 20) 
option(BUILD_TESTS "build tests with gtest" OFF)

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

if (BUILD_TESTS)
    find_package(PkgConfig)
    pkg_check_modules(GTEST REQUIRED gtest)
    pkg_check_modules(GTEST_MAIN REQUIRED gtest_main)
    add_executable(messenger-tests test/messengerTest.cpp)
    target_link_libraries(messenger-tests PUBLIC messenger gtest gtest_main)
    target_link_libraries(messenger-tests PRIVATE -fsanitize=address)
endif()
