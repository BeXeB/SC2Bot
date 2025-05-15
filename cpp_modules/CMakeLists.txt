cmake_minimum_required(VERSION 3.25.1)
project(sc2modules)

set(CMAKE_CXX_STANDARD 20)


if(WIN32)
    set(CMAKE_CXX_FLAGS -static)
endif()

set(STATE_SOURCE "${PROJECT_SOURCE_DIR}/src/sc2_state")
set(MCTS_SOURCE "${PROJECT_SOURCE_DIR}/src/mcts")
set(MODULE_SOURCE "${PROJECT_SOURCE_DIR}/src/module")
set(GRAPHS_SOURCE "${PROJECT_SOURCE_DIR}/src/graphs")
set(BENCHMARK_SOURCE "${PROJECT_SOURCE_DIR}/src/benchmarks")
set(TEST_SOURCE "${PROJECT_SOURCE_DIR}/src/tests")
set(EXTERNALS "${PROJECT_SOURCE_DIR}/externals")

# torchlib
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_PREFIX_PATH ${EXTERNALS}/libtorch/debug/libtorch)
    else ()
        set(CMAKE_PREFIX_PATH ${EXTERNALS}/libtorch/release/libtorch)
    endif()
else ()
    set(CMAKE_PREFIX_PATH ${EXTERNALS}/libtorch/linux/libtorch)
endif ()

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


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
        ${STATE_SOURCE}/enemy/enemy.cpp
)


target_include_directories(
        sc2_mcts
        PRIVATE ${MODULE_SOURCE}
        PRIVATE ${STATE_SOURCE}
        PRIVATE ${MCTS_SOURCE}
        PRIVATE ${STATE_SOURCE}/enemy
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

add_executable(MCTS_graphs
        ${GRAPHS_SOURCE}/graph_data.cpp
        ${STATE_SOURCE}/Sc2State.cpp
        ${STATE_SOURCE}/enemy/enemy.cpp
        ${STATE_SOURCE}/Construction.cpp
        ${MCTS_SOURCE}/Mcts.cpp
)

target_include_directories(
        MCTS_graphs
        PRIVATE ${STATE_SOURCE}
        PRIVATE ${STATE_SOURCE}/enemy
        PRIVATE ${MCTS_SOURCE}
)
## torchlib
target_link_libraries(sc2_mcts PRIVATE "${TORCH_LIBRARIES}")
target_link_libraries(RunTests PRIVATE "${TORCH_LIBRARIES}")
target_link_libraries(MCTS_benchmark PRIVATE "${TORCH_LIBRARIES}")

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    foreach (target sc2_mcts RunTests MCTS_benchmark)
        add_custom_command(TARGET ${target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${TORCH_DLLS}
                $<TARGET_FILE_DIR:${target}>)
    endforeach ()
endif (MSVC)

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

