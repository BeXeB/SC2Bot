cmake_minimum_required(VERSION 3.25.1)
project(sc2modules)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    set(CMAKE_CXX_FLAGS -static)
endif()

set(STATE_SOURCE "${PROJECT_SOURCE_DIR}/src/sc2_state")
set(MCTS_SOURCE "${PROJECT_SOURCE_DIR}/src/mcts")
set(MODULE_SOURCE "${PROJECT_SOURCE_DIR}/src/module")
set(BENCHMARK_SOURCE "${PROJECT_SOURCE_DIR}/src/benchmarks")
set(TEST_SOURCE "${PROJECT_SOURCE_DIR}/src/tests")
set(EXTERNALS "${PROJECT_SOURCE_DIR}/externals")

# pybind11
add_subdirectory(${EXTERNALS}/pybind11)

# doctests
add_executable(RunTests
        ${TEST_SOURCE}/Sc2StateTest.cpp
        ${TEST_SOURCE}/MctsTest.cpp
        ${STATE_SOURCE}/Sc2State.cpp
        ${STATE_SOURCE}/Construction.cpp
        ${MCTS_SOURCE}/Mcts.cpp
        ${STATE_SOURCE}/enemy/Enemy.cpp
)

target_include_directories(
        RunTests
        PRIVATE ${STATE_SOURCE}
        PRIVATE ${MCTS_SOURCE}
        PRIVATE "${STATE_SOURCE}/enemy"
)

# module
pybind11_add_module(
        sc2_mcts
        ${MODULE_SOURCE}/Module.cpp
        ${STATE_SOURCE}/Sc2State.cpp
        ${STATE_SOURCE}/Construction.cpp
        ${MCTS_SOURCE}/Mcts.cpp
)


target_include_directories(
        sc2_mcts
        PRIVATE ${MODULE_SOURCE}
        PRIVATE ${STATE_SOURCE}
        PRIVATE ${MCTS_SOURCE}
)

## executables
add_executable(MCTS_benchmark
        ${BENCHMARK_SOURCE}/mcts_benchmark.cpp
        ${STATE_SOURCE}/Sc2State.cpp
        ${STATE_SOURCE}/enemy/enemy.cpp
        ${STATE_SOURCE}/Construction.cpp
        ${MCTS_SOURCE}/Mcts.cpp
)

target_include_directories(
        MCTS_benchmark
        PRIVATE ${STATE_SOURCE}
        PRIVATE ${STATE_SOURCE}/enemy
        PRIVATE ${MCTS_SOURCE}
)

#add_executable(
#        sc2State
#        ${STATE_SOURCE}/Sc2State.cpp
#)
#
#target_include_directories(
#        sc2State
#        PRIVATE ${APP_SOURCE}
#)
#
#target_link_libraries(
#        sc2State
#        PRIVATE pybind11::embed
#)

