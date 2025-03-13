# Base image
FROM aiarena/arenaclient-bot:v0.6.10

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libstdc++-12-dev \
    python3 \
    python3-dev \
    python3-pip \
    wget \
    && apt-get clean

# Install CMake 3.29.0
RUN wget https://github.com/Kitware/CMake/releases/download/v3.29.0/cmake-3.29.0-linux-x86_64.sh && \
    chmod +x cmake-3.29.0-linux-x86_64.sh && \
    ./cmake-3.29.0-linux-x86_64.sh --skip-license --prefix=/usr/local

# Set working directory
WORKDIR /app

# Copy project files into the container
COPY . .

# Ensure C++20 is properly enabled
RUN echo "set(CMAKE_CXX_STANDARD 20)" >> /app/CMakeLists.txt && \
    echo "set(CMAKE_CXX_FLAGS \"\${CMAKE_CXX_FLAGS} -std=c++20\")" >> /app/CMakeLists.txt

RUN echo "deb http://deb.debian.org/debian testing main" >> /etc/apt/sources.list

RUN apt update \
    && apt install -y gcc-13 g++-13

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

RUN CXX=g++-13 CC=gcc-13 cmake -S . -B vsstudio \
    && cmake --build vsstudio --target sc2_mcts
