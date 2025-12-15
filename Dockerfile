FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy built executables
COPY build/casino_game /app/
COPY build/test_game /app/

# Set executable permissions
RUN chmod +x /app/casino_game /app/test_game

# Set environment variable for non-interactive mode
ENV JENKINS_HOME=/tmp

# Add health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD test -f /app/casino_game || exit 1

# Default command
CMD ["/app/casino_game"]
