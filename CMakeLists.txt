cmake_minimum_required(VERSION 3.7)
project(mathlogic)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES expr.h expr.cpp parser.h parser.cpp common.h proof.h proof.cpp)
add_executable(hw1 hw1.cpp ${SOURCE_FILES})
add_executable(hw2 hw2.cpp ${SOURCE_FILES})
add_executable(hw3 hw3.cpp ${SOURCE_FILES})