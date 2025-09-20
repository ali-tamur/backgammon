# setup.sh
#!/bin/bash

# Define the volume name
VOLUME_NAME="backgammon-data"

# Check if the volume already exists
if ! docker volume inspect "$VOLUME_NAME" > /dev/null 2>&1; then
    echo "Creating Docker volume: $VOLUME_NAME"
    docker volume create "$VOLUME_NAME"
else
    echo "Docker volume '$VOLUME_NAME' already exists."
fi

echo "Setup complete. You can now use 'docker compose up'."
