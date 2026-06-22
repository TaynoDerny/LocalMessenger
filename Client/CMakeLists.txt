cmake_minimum_required(VERSION 3.16)
project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON) # <-- 1. ВКЛЮЧАЕМ ПОДДЕРЖКУ РЕСУРСОВ

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

include_directories(src)

add_executable(Client
    src/main.cpp
    src/network/MessengerClient.cpp
    src/network/MessengerClient.h
    src/ui/auth/AuthWindow.cpp
    src/ui/auth/AuthWindow.h
    src/ui/chat/ChatWindow.cpp
    src/ui/chat/ChatWindow.h
    src/ui/admin/CreateUserDialog.cpp
    src/ui/admin/CreateUserDialog.h
    resources.qrc         # <-- 2. ДОБАВЛЯЕМ ФАЙЛ РЕСУРСОВ В СБОРКУ
)

target_link_libraries(Client PRIVATE Qt6::Core Qt6::Network Qt6::Widgets)