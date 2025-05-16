#!/bin/sh
set -e  # Exit immediately if a command exits with a non-zero status.

# Compile the program using the latest source files
cmake -S /app/cpp_modules -B /app/cpp_modules/docker-build
cmake --build /app/cpp_modules/docker-build --target sc2_mcts

mv /app/cpp_modules/docker-build/sc2_mcts* /app