# Base image
FROM aiarena/arenaclient-bot:v0.6.10

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    # libstdc++-12-dev \
    cmake \
    && apt-get clean


# Set working directory
WORKDIR /app

# Copy project files into the container
COPY . .

RUN cmake -S . -B vsstudio \
    && cmake --build vsstudio --target sc2_mcts
