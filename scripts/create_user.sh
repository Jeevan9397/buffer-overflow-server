#!/usr/bin/env bash
# Usage: ./scripts/create_user.sh

# Resolve this script’s own directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_FILE="${SCRIPT_DIR}/../data/auth.csv"

mkdir -p "$(dirname "$DATA_FILE")"

read -p "Username: " username
while [[ -z "$username" ]]; do
  echo "Username cannot be empty."
  read -p "Username: " username
done

read -p "Role (admin/user): " role
while [[ "$role" != "admin" && "$role" != "user" ]]; do
  echo "Role must be 'admin' or 'user'."
  read -p "Role (admin/user): " role
done

read -sp "Password: " password; echo
minlen=$([[ "$role" == "admin" ]] && echo 8 || echo 6)
while (( ${#password} < minlen )); do
  echo "Password must be at least $minlen characters."
  read -sp "Password: " password; echo
done

# Compute the hex hash without newlines
 hash=$(printf '%s' "$password" | openssl dgst -sha256 | awk '{print $2}')

# append to auth.csv
echo "${username},${hash},${role}" >> "$DATA_FILE"
echo "Created user '$username' with role '$role'."
