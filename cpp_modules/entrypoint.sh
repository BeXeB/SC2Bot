#!/bin/sh
set -e  # Exit immediately if a command exits with a non-zero status.

# Compile the program using the latest source files
cmake -S /app -B /app/docker-build
cmake --build /app/docker-build --target sc2_mcts