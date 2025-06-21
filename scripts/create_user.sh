#!/usr/bin/env bash
#
# Usage: ./scripts/create_user.sh
#   Prompts for: username, role [admin/user], password
#   Enforces min-length (8 for admin, 6 for user), then
#   SHA-256-hashes the password and appends "user,hash,role" to data/auth.csv

DATA_FILE="$(dirname "$0")/../data/auth.csv"
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

# compute sha256 hash
hash=$(printf '%s' "$password" | openssl dgst -binary -sha256 | xxd -p)

# append to auth.csv
echo "${username},${hash},${role}" >> "$DATA_FILE"
echo "Created user '$username' with role '$role'."

