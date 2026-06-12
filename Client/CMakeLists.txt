cmake_minimum_required(VERSION 3.16)
project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Network REQUIRED)

include_directories(src)

add_executable(Client
    main.cpp
    src/MessengerClient.cpp
)

target_link_libraries(Client PRIVATE Qt6::Core Qt6::Network)