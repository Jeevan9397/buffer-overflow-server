# 🛡️ Buffer Overflow Server (C++ + AngularJS)

A lightweight customer management server demonstrating full CRUD functionality, CSV import/export, and secure buffer overflow handling — written in C++ (Crow HTTP) with a web-based AngularJS frontend.

---

## 📦 Features

- 🔐 Login authentication using SHA256
- 📁 Customer CRUD: Create, Read, Update, Delete
- 📤 Export selected/full records to CSV
- 📥 Import customer data from CSV (escaped with `"` support)
- 🧪 Buffer Overflow demonstration + mitigation
- 📊 Responsive AngularJS frontend UI
- 🐧 Designed for Linux (RHEL 9, Ubuntu 20.04+)

---

## 🚀 Setup (Linux VM / RHEL 9 / Ubuntu)

### 1. Prerequisites

```bash
sudo dnf update -y
sudo dnf groupinstall "Development Tools" -y
sudo dnf install cmake git gdb valgrind openssl-devel -y


Buffer Overflow Vulnerability Demonstration

Overview

This project demonstrates real-world memory corruption vulnerabilities such as:

Stack-Based Buffer Overflow

Format String Vulnerability

Integer Overflow

Heap Overflow

Off-by-One Error

Built with:

C++ backend using Crow Framework

AngularJS frontend

Vulnerabilities injected into backend routes for demo

Project Structure

buffer_overflow/
├── src/                  # main.cpp (safe), main_bof.cpp (vulnerable)
├── include/              # crow and dependencies
├── UI/                   # AngularJS frontend
├── data/                 # customer.csv, auth.csv
├── build/                # build output
├── app.log               # log file
├── CMakeLists.txt        # build config

Setup Instructions

Prerequisites

RHEL9 / Ubuntu

GCC / G++

cmake, make

valgrind, gdb

Build Safe Version

cd build
cmake ..
make
./app-server

Build Vulnerable Version

cp src/main_bof.cpp src/main.cpp
cd build
make clean && make
./bof-server

Demonstrated Attacks

1. Stack-Based Buffer Overflow

Route: POST /api/customers

Code: strcpy(buffer, name.c_str());

Test: Input name > 32 chars

Crash: Yes (stack corruption)

Fix: Use strncpy() or std::string

2. Format String Vulnerability

Route: POST /api/customers

Code: snprintf(buf, sizeof(buf), input.c_str());

Test: Input: AAAA%s%x%n

Leak: Yes (stack values exposed)

Fix: Format strings must be constants

3. Integer Overflow + Heap Overflow

Route: POST /api/import

Code: new char[length * 100000000];

Test: Import CSV with large field

Crash: Yes (integer wrap -> bad allocation)

Fix: Check before multiplication

4. Off-by-One Error

Route: PUT /api/customers/<id>

Code: strncpy(buf, input, buf+1)

Test: 33-char name to 32-byte buffer

Result: Corrupts adjacent memory (canary)

Fix: Limit strncpy, log & monitor

Tooling

valgrind for memory errors

gdb for crash debugging

curl or Postman for testing

Example:

curl -X POST http://localhost:8080/api/customers -d '{"name": "AAAA..."}'

Logs (app.log)

Every request & action logged

Crashes / overflows logged with details

Example:

2025-07-10 00:09:41 Off-by-one address buffer + canary + overflow: magdeburgnnnn...sssssss

Defensive Hardening Summary

Attack Type

Mitigation

Stack Overflow

Bounds check, strncpy, std::string

Format String

Constant format strings only

Integer Overflow

Pre-calculate & check before allocating

Heap Overflow

Validate sizes before new[]

Off-by-One

Careful buffer management + logging

Demo Strategy

Two binaries: app-server (safe), bof-server (vulnerable)

Frontend + backend logs

Attack via UI / Postman / curl

Documented logs, screenshots, crash reports

Author

Developed by JeevanFor academic demonstration and secure coding education.

