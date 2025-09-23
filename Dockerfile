# Start with a base Python image.
FROM nvidia/cuda:12.2.0-runtime-ubuntu22.04

# Install Python and other necessary tools.
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3.12 \
    python3-pip \
    python3-setuptools \
    && apt-get clean && rm -rf /var/lib/apt/lists/*


# Set the working directory for your application.
WORKDIR /app

# Create a non-root user and group to run the application.
RUN addgroup --gid 1001 backgammon_group && adduser --system --uid 1001 --ingroup backgammon_group backgammon_user

# Copy requirements.txt first to leverage Docker's layer caching.
# This only runs when requirements.txt changes.
COPY requirements.txt .

# Install dependencies. This step is performed as the root user.
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the application code.
COPY . .

# Create the data directory for the Docker volume.
RUN mkdir -p /app/data

# Change ownership of the entire application and data directories to the non-root user.
RUN chown -R backgammon_user:backgammon_group /app

# Set the correct permissions for the data directory.
RUN chmod -R 775 /app/data

# Switch to the non-root user for all subsequent commands and for runtime.
USER backgammon_user

# Set the default command to keep the container running.
CMD ["sleep", "infinity"]
