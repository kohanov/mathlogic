cmake_minimum_required(VERSION 3.7)
project(mathlogic)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES expr.h parser.cpp parser.h)
add_executable(hw1 hw1.cpp ${SOURCE_FILES})
add_executable(hw2 hw2.cpp ${SOURCE_FILES})