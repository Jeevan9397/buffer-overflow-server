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
