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

# Ensure the entrypoint script is executable
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]