cmake_minimum_required(VERSION 3.29)
project(cppXpython)

set(CMAKE_CXX_STANDARD 20)

add_executable(Doctest_tests_run Sc2StateTest.cpp)

#target_link_libraries(Doctest_tests_run sc2State)