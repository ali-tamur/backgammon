# Use an official Python runtime as a parent image
FROM python:3.12-slim

# Set the working directory in the container
WORKDIR /app

# Copy the requirements.txt file from your project to the container's /app directory
COPY requirements.txt .

# Install any dependencies specified in requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of your project's code to the container's /app directory
COPY . .

RUN mkdir -p logs && touch logs/mylog.avro

# Specify the command to run when the container starts
CMD ["python3", "-m", "src.backgammon"]
