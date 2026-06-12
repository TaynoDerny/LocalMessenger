cmake_minimum_required(VERSION 3.16)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON) # Важно: включает поддержку сигналов и слотов в наших классах

# Подключаем нужные модули Qt
find_package(Qt6 COMPONENTS Core Network Sql REQUIRED)

# Указываем, где искать заголовочные файлы
include_directories(src)

# Указываем все наши файлы для сборки
add_executable(Server
    main.cpp
    src/DatabaseManager.cpp
    src/MessengerServer.cpp
)

target_link_libraries(Server PRIVATE Qt6::Core Qt6::Network Qt6::Sql)