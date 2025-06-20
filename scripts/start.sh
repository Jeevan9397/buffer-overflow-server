#!/usr/bin/env bash
# Launch the server in the background and tail logs
BASE_DIR="$(dirname "$0")/.."
cd "$BASE_DIR"

PID_FILE="server.pid"
LOG_FILE="app.log"

if [ -f "$PID_FILE" ]; then
  echo "Server already running (PID $(cat $PID_FILE))"
  exit 1
fi

# Start server and redirect output to both console and log
nohup ./build/app-server 2>&1 | tee -a "$LOG_FILE" &
echo $! > "$PID_FILE"
echo "Server started with PID $(cat $PID_FILE)"
