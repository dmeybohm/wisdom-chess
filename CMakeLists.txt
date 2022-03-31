set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomMultiModule CXX)

find_package(Qt6 6.2 COMPONENTS Quick)

add_subdirectory(engine)

if (Qt6_FOUND)
    add_subdirectory(qt-qml-gui)
endif()
