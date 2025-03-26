#!/bin/bash

# Start Nginx in the background
nginx

# Start Python FastAPI server in the foreground
# This will keep the container running and show logs
python3 /jpf/update.py
