cmake_minimum_required(VERSION 3.16)
project(LocalMessenger)

# Даем команду собрать обе части
add_subdirectory(Server)
add_subdirectory(Client)
