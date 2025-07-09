#!/usr/bin/env bash
# scripts/start.sh

# Resolve this script’s directory, then the project root:
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
BIN="$ROOT/build/app-server"
PIDFILE="$ROOT/server.pid"
LOGFILE="$ROOT/server-debug.log"

# 1) Tear down any existing instance
"$ROOT"/scripts/stop.sh

# 2) Move into the project root for correct relative paths
cd "$ROOT" || { echo " Could not cd to $ROOT"; exit 1; }

# 3) Launch the server in background, logging all output
nohup "$BIN" > "$LOGFILE" 2>&1 &
NEWPID=$!

# 4) Record the PID and report
echo $NEWPID > "$PIDFILE"
echo " Server started with PID $NEWPID (see $LOGFILE for logs)"

