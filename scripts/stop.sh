#!/usr/bin/env bash
#
# scripts/stop.sh
#

# 1) Resolve directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
PIDFILE="$ROOT/server.pid"

echo " Stopping server..."

# 2) Kill the PID in server.pid (if it exists)
if [[ -f "$PIDFILE" ]]; then
  PID=$(<"$PIDFILE")
  if kill -0 "$PID" &>/dev/null; then
    echo "   → Killing PID $PID from $PIDFILE"
    kill "$PID"
  else
    echo "   → No process $PID; skipping PIDFILE kill"
  fi
  rm -f "$PIDFILE"
fi

# 3) Kill any stray app-server processes under the build folder
PIDS=$(pgrep -f "$ROOT/build/app-server" || true)
if [[ -n "$PIDS" ]]; then
  echo "   → Killing stray app-server processes: $PIDS"
  kill $PIDS
fi

echo " Server stopped."
