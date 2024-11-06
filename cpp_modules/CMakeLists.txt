cmake_minimum_required(VERSION 3.29)
project(sc2modules)

set(CMAKE_CXX_STANDARD 20)

set(STATE_SOURCE "${PROJECT_SOURCE_DIR}/src/sc2_state")
set(MODULE_SOURCE "${PROJECT_SOURCE_DIR}/src/module")
set(TEST_SOURCE "${PROJECT_SOURCE_DIR}/src/tests")
set(EXTERNALS "${PROJECT_SOURCE_DIR}/externals")

# pybind11
add_subdirectory(${EXTERNALS}/pybind11)

# doctests
#add_subdirectory(src/tests)
add_executable(Sc2StateTest
        ${TEST_SOURCE}/Sc2StateTest.cpp
        ${STATE_SOURCE}/Sc2State.cpp
)

target_include_directories(
        Sc2StateTest
        PRIVATE ${STATE_SOURCE}
)

# module
pybind11_add_module(
        sc2_mcts
        ${MODULE_SOURCE}/Module.cpp
        ${STATE_SOURCE}/Sc2State.cpp
)

target_include_directories(
        sc2_mcts
        PRIVATE ${MODULE_SOURCE}
        PRIVATE ${STATE_SOURCE}
)

## executables
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

