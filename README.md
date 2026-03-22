# 💬 TCP Chat Application (C++)

A real-time multi-client chat application built using C++ and TCP sockets with support for private messaging, user management, and command-based interaction.

---

## 🚀 Overview

This project implements a client-server chat system using low-level socket programming in C++.
It supports multiple users communicating simultaneously with features like private chat, user listing, and command handling.

The system focuses on:

* Networking fundamentals
* Multithreading
* System design
* Improving user experience in terminal-based applications

---

## 🧠 Features

### 🔹 Core Features

* Multi-client chat system using TCP sockets
* Real-time communication using client-server architecture
* Thread-based handling of multiple clients

### 🔹 User Interaction

* Unique username enforcement
* Join/leave notifications
* Online user listing (`/list`)

### 🔹 Messaging Features

* Public chat (broadcast messages)
* Private messaging (`/msg <user> <message>`)
* Persistent private chat mode (`/chat <user>`)

### 🔹 System Enhancements

* Timestamped messages
* Command-based interface
* Error handling for invalid commands
* Improved terminal UI for concurrent input/output

---

## 🧾 Commands

| Command                 | Description                     |
| ----------------------- | ------------------------------- |
| `/msg <user> <message>` | Send a private message          |
| `/chat <user>`          | Enter private chat mode         |
| `/list`                 | Show all online users           |
| `/help`                 | Display all available commands  |
| `/exit`                 | Exit private chat or disconnect |

---

## 🏗️ Project Structure

```
.
├── main_server.cpp   # Server-side implementation
├── main_client.cpp   # Client-side implementation
├── README.md
```

---

## ⚙️ Technologies Used

* C++
* Winsock (Windows Sockets API)
* Multithreading (`std::thread`)
* MSYS2 / MinGW (for compilation)

---

## 🧪 How to Run

### 🔹 Compile

```
g++ main_server.cpp -o server.exe -lws2_32 -std=c++17 -pthread
g++ main_client.cpp -o client.exe -lws2_32 -std=c++17 -pthread
```

---

### 🔹 Run Server

```
./server.exe
```

---

### 🔹 Run Clients (multiple terminals)

```
./client.exe
```

---

## 🖥️ Sample Interaction

```
[11:32] Anup joined the chat
[11:33] Kanak joined the chat

You: Hello everyone
[11:34] Kanak: Hi!

You: /chat Kanak
[Private chat with Kanak]

You: How are you?
[11:35] [PRIVATE] Kanak: I'm good!
```

---

## 📌 Key Concepts Demonstrated

* TCP socket programming
* Client-server architecture
* Multithreading and concurrency
* Synchronization using mutex
* Command parsing and protocol design
* Real-time system design

---

## 🔖 Version

**v1.0 — Feature Complete Backend**

* Core chat system implemented
* Command handling stabilized
* Ready for GUI integration

---

## 🚀 Future Improvements

* GUI integration (Qt / SFML)
* Chat rooms / channels
* Message history
* File transfer
* Encryption
* Reconnection handling

---

## 👨‍💻 Author

Anup Kumar Mishra

---

## ⭐ Notes

This project demonstrates practical understanding of:

* Low-level networking
* Concurrent programming
* Building real-world systems in C++

---
