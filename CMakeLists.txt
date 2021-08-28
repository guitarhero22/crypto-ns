cmake_minimum_required(VERSION 3.0)

project(crypto-ns VERSION 1.0)

add_executable(crypto-ns src/main.cpp src/utils.cpp src/simulator.cpp src/event.cpp src/peer.cpp src/block.cpp)

target_include_directories(crypto-ns PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

