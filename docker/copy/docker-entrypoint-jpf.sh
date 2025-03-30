#!/bin/bash
set -e

echo "Starting services..."

# Start Nginx in the background
echo "Starting Nginx..."
nginx
echo "Nginx started"

# Check if Nginx started successfully
if ! pgrep -x "nginx" > /dev/null; then
    echo "Error: Nginx failed to start"
    exit 1
fi

# Function to handle container shutdown
function cleanup() {
    echo "Shutting down services..."
    nginx -s stop
    kill -TERM "$PY_PID" 2>/dev/null || true
    echo "All services stopped"
    exit 0
}

# Set up signal trapping
trap cleanup SIGTERM SIGINT

# Start Python FastAPI server in the background
echo "Starting Python application..."
python3 /jpf/update.py &
PY_PID=$!

# Wait for Python process to finish
wait $PY_PID
