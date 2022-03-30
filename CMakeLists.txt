set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomMultiModule CXX)

add_subdirectory(engine)

# Todo - make gui able to be disabled
add_subdirectory(qt-qml-gui)
