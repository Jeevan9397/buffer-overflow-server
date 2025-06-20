#!/usr/bin/env bash
# Stop the server using the PID file
BASE_DIR="$(dirname "$0")/.."
cd "$BASE_DIR"

PID_FILE="server.pid"

if [ ! -f "$PID_FILE" ]; then
  echo "No PID file found. Is the server running?"
  exit 1
fi

kill "$(cat $PID_FILE)" && rm "$PID_FILE"
echo "Server stopped."
